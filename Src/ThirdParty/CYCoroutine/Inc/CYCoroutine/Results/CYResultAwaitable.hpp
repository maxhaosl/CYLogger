#ifndef __CY_RESULT_AWAITABLE_CORO_HPP__
#define __CY_RESULT_AWAITABLE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/Impl/CYResultState.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<class TYPE>
class CYAwaitableBase : public suspend_always
{
public:
    CYAwaitableBase(CYConsumerResultStatePtr<TYPE> ptrState) noexcept
        : m_ptrState(std::move(ptrState))
    {
    }

protected:
    CYConsumerResultStatePtr<TYPE> m_ptrState;

private:
    CYAwaitableBase(const CYAwaitableBase&) = delete;
    CYAwaitableBase(CYAwaitableBase&&) = delete;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYAwaitable : public CYAwaitableBase<TYPE>
{

public:
    CYAwaitable(CYConsumerResultStatePtr<TYPE> ptrState) noexcept
        : CYAwaitableBase<TYPE>(std::move(ptrState))
    {
    }

    bool await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        assert(static_cast<bool>(this->m_ptrState));
        return this->m_ptrState->await(handleCaller);
    }

    TYPE await_resume()
    {
        CYJoinedConsumerResultStatePtr<TYPE> ptrState(this->m_ptrState.release());
        return ptrState->Get();
    }
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYResult;

template<class TYPE>
class CYResolveAwaitable : public CYAwaitableBase<TYPE>
{

public:
    CYResolveAwaitable(CYConsumerResultStatePtr<TYPE> ptrState) noexcept
        : CYAwaitableBase<TYPE>(std::move(ptrState))
    {
    }

    bool await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        assert(static_cast<bool>(this->m_ptrState));
        return this->m_ptrState->await(handleCaller);
    }

    CYResult<TYPE> await_resume()
    {
        return CYResult<TYPE>(std::move(this->m_ptrState));
    }

private:
    CYResolveAwaitable(CYResolveAwaitable&&) noexcept = delete;
    CYResolveAwaitable(const CYResolveAwaitable&) noexcept = delete;
};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_RESULT_AWAITABLE_CORO_HPP__