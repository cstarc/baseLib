
#ifndef CHX_QUEUE_WITH_LOCK_FREE_H_
#define CHX_QUUEU_WITH_LOKC_FREE_H_

#include<atomic>


#include "hazard_pointer.h"



namespace chx
{

    template<typename T>
    class Queue
    {
    public:
        virtual bool Enqueue(T const & value)=0;
        virtual bool Dequeue(T & value)=0;
    };


//*************************************************************
//                      hazard pointer
//*************************************************************

    template<typename T>
    class QueueWithHazardPointer: public Queue<T>
    {
    public:
        struct Node
        {
            T data;
            atomic<Node*> next;
            Node():next(nullptr){}
        };
        QueueWithHazardPointer();
        virtual bool Enqueue(T const & value)  override final;
        virtual bool Dequeue(T& value) override final;

    private:
        atomic<Node*> m_head;
        atomic<Node*> m_tail;
        HazardPointManager<Node> m_hp_manager;
    };

    template<typename T>
    QueueWithHazardPointer<T>::QueueWithHazardPointer()
    {
        m_head=m_tail=new Node();
    }

    template<typename T>
    bool QueueWithHazardPointer<T>::Enqueue(T const & value)
    {
        Node* new_node=new Node();
        new_node->data=value;

        HazardPointerData<Node>& hp_data =m_hp_manager.AllocatorHP();
        while(true)
        {
            Node* tail=m_tail.load(memory_order::memory_order_acquire);

            hp_data.hazard_pointer[0]=tail;

            if( unlikely(tail!=m_tail.load(memory_order::memory_order_relaxed)) )
                continue;

            Node* tail_next=tail->next;

            if( unlikely(tail_next!=nullptr) )
            {
                m_tail.compare_exchange_strong(tail,tail_next,memory_order::memory_order_release);
                continue;
            }

            if( likely(tail->next.compare_exchange_weak(tail_next,new_node,memory_order::memory_order_release)) )
            {
                m_tail.compare_exchange_strong(tail,new_node,memory_order::memory_order_release);
                break;
            }

        }
        hp_data.hazard_pointer[0]=nullptr;
        m_hp_manager.ReleaseHP(hp_data);
        return true;
    }

    template<typename T>
    bool QueueWithHazardPointer<T>::Dequeue(T& value)
    {
        HazardPointerData<Node>& hp_data = m_hp_manager.AllocatorHP();
        bool is_empty=false;
        while(true)
        {
            Node* head=m_head.load(memory_order_acquire);
            hp_data.hazard_pointer[0]=head;

            //主要作用:有可能在把head放入hazard前，head已被删除改变，故需判断，以保证head存在
            if(unlikely(head != m_head.load(memory_order_acquire)))
                continue;

            Node* tail=m_tail.load(memory_order_acquire);
            Node* next=head->next.load(memory_order_acquire);
            hp_data.hazard_pointer[1]=next;

            // 若相等，保证了head->next==m_head->next,next 存在
            if(unlikely(head != m_head.load(memory_order_acquire)))
                continue;

            if(next==nullptr)
            {
                is_empty=true;
                break;
            }
            if(head==tail)
            {
                //线程互助？
                m_tail.compare_exchange_strong(tail,next);
                continue;
            }

            if(m_head.compare_exchange_weak(head,next))
            {
                value=next->data;

                hp_data.hazard_pointer[0]=nullptr;
                hp_data.hazard_pointer[1]=nullptr;
                m_hp_manager.RetireNode(hp_data,head);
                m_hp_manager.ReleaseHP(hp_data);
                break;
            }
        }

        if(is_empty)
            return false;
        return true;


    }

}

#endif // QUEUE_WITH_LOCK_FREE
