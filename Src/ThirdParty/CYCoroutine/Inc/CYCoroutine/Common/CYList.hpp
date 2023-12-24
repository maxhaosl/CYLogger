#ifndef ___CY_LIST_CORO_HPP___
#define ___CY_LIST_CORO_HPP___

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include <cassert>

CYCOROUTINE_NAMESPACE_BEGIN

template<class NODE_TYPE>
class CYList
{
public:
    CYList() noexcept = default;

    CYList(CYList& rhs) noexcept
        : m_pHead(rhs.m_pHead)
        , m_pTail(rhs.m_pTail)
    {
        rhs.m_pHead = nullptr;
        rhs.m_pTail = nullptr;
    }

    CYList(CYList&& rhs) noexcept
        : m_pHead(rhs.m_pHead)
        , m_pTail(rhs.m_pTail)
    {
        rhs.m_pHead = nullptr;
        rhs.m_pTail = nullptr;
    }

public:
    bool Empty() const noexcept
    {
        CheckState();
        return m_pHead == nullptr;
    }

    void PushBack(NODE_TYPE& node) noexcept
    {
        CheckState();

        if (m_pHead == nullptr)
        {
            m_pHead = m_pTail = &node;
            return;
        }

        m_pTail->next = &node;
        m_pTail = &node;
    }

    NODE_TYPE* PopFront() noexcept
    {
        CheckState();
        const auto node = m_pHead;
        if (node == nullptr)
        {
            return nullptr;
        }

        m_pHead = m_pHead->next;
        if (m_pHead == nullptr)
        {
            m_pTail = nullptr;
        }

        return node;
    }

private:
    NODE_TYPE* m_pHead = nullptr;
    NODE_TYPE* m_pTail = nullptr;

    void CheckState() const noexcept
    {
        if (m_pHead == nullptr)
        {
            assert(m_pTail == nullptr);
            return;
        }

        assert(m_pTail != nullptr);
    }
};

CYCOROUTINE_NAMESPACE_END

#endif //___CY_LIST_CORO_HPP___