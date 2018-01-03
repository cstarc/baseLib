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
                // ����������Ͽ����ӵ��½ڵ�û�ͷŵ������Ҳ�ᴦ�������Ͽ����ڵ㻹û�ͷŵ������������ʵ������Ҫ����
                // ��ֻ��scan�����,��С���ģ�
                HelpScan(hp);
            }
        }

    private:
        void Scan(HazardPointerData<T>& cur_hp_data)
        {

            //1. ��ȡ��ʹ�õ�node��list
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

            //2. ɾ��δ�� used_list , ���� retire_list �е� node
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
            //* ��� inactive ���� retire_list ��Ϊ�յ�hp�����뵱ǰ��retire_list
            //**
            for(HazardPointerData<T>& hp_rec : m_hp_Array)
            {
                if(hp_rec.is_active.test_and_set(memory_order_acquire))
                    continue;

                // ��֤��hp_recֻ�е�ǰ�߳���д
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
