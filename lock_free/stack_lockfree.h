#ifndef STACK_LOCKFREE_
#define STACK_LOCKFREE_

#include "hazard_pointer.h"

namespace chx
{

   template<typename T>
    class StackWithHazardPointer
    {
    public:
        struct Node
        {
            T data;
            atomic<Node*> next;
            Node():next(nullptr){}
        };
        StackWithHazardPointer();
        bool Push(T const & value);
        bool Pop(T& value);

    private:
        atomic<Node*> m_head;
        HazardPointManager<Node> m_hp_manager;
    };

    template<typename T>
    StackWithHazardPointer<T>::StackWithHazardPointer():m_head(nullptr)
    {
    }

    template<typename T>
    bool StackWithHazardPointer<T>::Push(T const & value)
    {
        Node* new_node=new Node();
        new_node->data=value;

        while(true)
        {
            Node* head=m_head.load(memory_order::memory_order_acquire);

            new_node->next.store(head,memory_order::memory_order_release);

            if(m_head.compare_exchange_strong(head,new_node,memory_order::memory_order_release))
                return true;

        }
        return true;
    }

    template<typename T>
    bool StackWithHazardPointer<T>::Pop(T& value)
    {
        HazardPointerData<Node>& hp_data = m_hp_manager.AllocatorHP();
        bool is_empty=false;
        while(true)
        {
            Node* head=m_head.load(memory_order_acquire);

            if(head==nullptr)
            {
                is_empty=true;
                break;
            }

            hp_data.hazard_pointer[0]=head;

            //主要作用:有可能在把head放入hazard前，head已被删除改变，故需判断，以保证head存在
            if(unlikely(head != m_head.load(memory_order_acquire)))
                continue;

            Node* next=head->next.load(memory_order_acquire);

            if(m_head.compare_exchange_weak(head,next))
            {
                value=head->data;
                delete head;
                hp_data.hazard_pointer[0]=nullptr;
                m_hp_manager.RetireNode(hp_data,head);
                m_hp_manager.ReleaseHP(hp_data);
                break;
            }
        }

        if(is_empty)
            return false;
        return true;


    }

}  // namespace chx
#endif // STACK_LOCKFREE_
