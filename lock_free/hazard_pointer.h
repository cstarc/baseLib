#ifndef HAZARD_POINTER_
#define HAZARD_POINTER_

#include<atomic>
#include<vector>
#include "../include/global.h"

namespace chx
{
//*************************************************************
//                      hazard pointer
//*************************************************************

    #define MAX_TID 5000
    #define RETIRE_THRESHOLD (2*MAX_TID)

    template <typename T>
    struct HazardPointerData
    {
        atomic_flag is_active   = ATOMIC_FLAG_INIT;
        T* hazard_pointer[2] = {nullptr,nullptr};
        //void* retire_list[RETIRE_THRESHOLD];
        vector<T*> retire_list;
        int retire_count        = 0;

    };

    template <typename T>
    class HazardPointManager
    {
    public:
        HazardPointerData<T>& AllocatorHP()
        {
            for(int i=0;i<MAX_TID;++i)
            {
                if(!m_hp_Array[i].is_active.test_and_set(memory_order::memory_order_relaxed))
                    return m_hp_Array[i];
            }
            assert(0);
        }

        void ReleaseHP(HazardPointerData<T>& hp)
        {
            hp.is_active.clear(memory_order::memory_order_relaxed);
        }

        void RetireNode(HazardPointerData<T>& hp,T* node)
        {
            //hp.retire_list[hp.retire_count++]=node;
            hp.retire_list.push_back(node);
            hp.retire_count++;

            if(hp.retire_count > RETIRE_THRESHOLD)
            {
                Scan(hp);
                // 处理因意外断开连接导致节点没释放的情况（也会处理正常断开，节点还没释放的情况，而这其实并不需要处理，
                // 故只在scan后调用,减小消耗）
                HelpScan(hp);
            }
        }

    private:
        void Scan(HazardPointerData<T>& cur_hp_data)
        {

            //1. 获取在使用的node的list
            T* used_list[2*MAX_TID];
            int used_count=0;
            for(HazardPointerData<T>& hp_rec : m_hp_Array)
            {
                for(auto hp : hp_rec.hazard_pointer)
                {
                    if(hp!=nullptr)
                    {
                        used_list[used_count++]=hp;
                    }
                }
            }

            //2. 删除未在 used_list , 但在 retire_list 中的 node
            vector<T*> tmp_retire;
            tmp_retire.swap(cur_hp_data.retire_list);
            cur_hp_data.retire_count=0;
            for(auto retire: tmp_retire)
            {
                if(find(retire,used_list,used_count))
                {
                    cur_hp_data.retire_list.push_back(retire);
                    cur_hp_data.retire_count++;
                }
                else
                {
                    delete retire; // delete void * ,maybe error(now is ok )
                    retire=nullptr;
                }
            }

        }

        void HelpScan(HazardPointerData<T>& cur_hp_data)
        {

            //**
            //* 检测 inactive 并且 retire_list 不为空的hp，放入当前的retire_list
            //**
            for(HazardPointerData<T>& hp_rec : m_hp_Array)
            {
                if(hp_rec.is_active.test_and_set(memory_order_acquire))
                    continue;

                // 保证此hp_rec只有当前线程在写
                while(hp_rec.retire_count>0)
                {
                    auto node=hp_rec.retire_list[--hp_rec.retire_count];
                    hp_rec.retire_list.pop_back();

                    cur_hp_data.retire_list.push_back(node);
                    cur_hp_data.retire_count++;

                    if(cur_hp_data.retire_count > RETIRE_THRESHOLD)
                        Scan(cur_hp_data);
                }

                hp_rec.is_active.clear(memory_order_release);
            }
        }

        bool find(T* node,T** used_list, int used_count)
        {
            for(int i=0;i<used_count;++i)
            {
                if(node==used_list[i])
                    return true;
            }

            return false;
        }

    private:
        HazardPointerData<T> m_hp_Array[MAX_TID];
    };

}

#endif // HAZARD_POINTER_
