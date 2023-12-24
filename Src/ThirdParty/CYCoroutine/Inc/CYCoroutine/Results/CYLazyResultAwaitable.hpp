#ifndef __CY_LAZY_RESULT_AWAITTABLE_CORO_HPP__
#define __CY_LAZY_RESULT_AWAITTABLE_CORO_HPP__

#include "CYCoroutine/Results/Impl/CYLazyResultState.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYLazyAwaitable
{
public:
    CYLazyAwaitable(coroutine_handle<CYLazyResultState<TYPE>> handleState) noexcept
        : m_handleState(handleState)
    {
        assert(static_cast<bool>(handleState));
    }

    virtual ~CYLazyAwaitable() noexcept
    {
        auto handleState = m_handleState;
        handleState.destroy();
    }

    bool await_ready() const noexcept
    {
        return m_handleState.done();
    }

    coroutine_handle<void> await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        return m_handleState.promise().await(handleCaller);
    }

    TYPE await_resume()
    {
        return m_handleState.promise().Get();
    }

private:
    CYLazyAwaitable(const CYLazyAwaitable&) = delete;
    CYLazyAwaitable(CYLazyAwaitable&&) = delete;

private:
    const coroutine_handle<CYLazyResultState<TYPE>> m_handleState;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYLazyResolveAwaitable
{
public:
    CYLazyResolveAwaitable(coroutine_handle<CYLazyResultState<TYPE>> handleState) noexcept
        : m_handleState(handleState)
    {
        assert(static_cast<bool>(handleState));
    }

    virtual ~CYLazyResolveAwaitable() noexcept
    {
        if (static_cast<bool>(m_handleState))
        {
            m_handleState.destroy();
        }
    }

    bool await_ready() const noexcept
    {
        return m_handleState.done();
    }

    coroutine_handle<void> await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        return m_handleState.promise().await(handleCaller);
    }

    CYLazyResult<TYPE> await_resume() noexcept
    {
        return { std::exchange(m_handleState, {}) };
    }

private:
    CYLazyResolveAwaitable(CYLazyResolveAwaitable&&) = delete;
    CYLazyResolveAwaitable(const CYLazyResolveAwaitable&) = delete;

private:
    coroutine_handle<CYLazyResultState<TYPE>> m_handleState;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_LAZY_RESULT_AWAITTABLE_CORO_HPP__