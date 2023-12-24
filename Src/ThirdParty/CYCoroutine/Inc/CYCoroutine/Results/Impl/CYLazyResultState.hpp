#ifndef __CY_LAZY_RESULT_STATE_CORO_HPP__
#define __CY_LAZY_RESULT_STATE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/Impl/CYProducerContext.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

struct CYLazyFinalAwaiter : public suspend_always
{
    template<class promise_type>
    coroutine_handle<void> await_suspend(coroutine_handle<promise_type> handle) noexcept
    {
        return handle.promise().ResumeCaller();
    }
};

//////////////////////////////////////////////////////////////////////////
class CYLazyResultStateBase
{
public:
    coroutine_handle<void> ResumeCaller() const noexcept
    {
        return m_handleCaller;
    }

    coroutine_handle<void> await(coroutine_handle<void> handleCaller) noexcept
    {
        m_handleCaller = handleCaller;
        return coroutine_handle<CYLazyResultStateBase>::from_promise(*this);
    }

protected:
    coroutine_handle<void> m_handleCaller;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYLazyResult;

template<class type>
class CYLazyResultState : public CYLazyResultStateBase
{
public:
    EResultStatus Status() const noexcept
    {
        return m_objProducer.Status();
    }

    CYLazyResult<type> get_return_object() noexcept
    {
        const auto self_handle = coroutine_handle<CYLazyResultState>::from_promise(*this);
        return CYLazyResult<type>(self_handle);
    }

    void unhandled_exception() noexcept
    {
        m_objProducer.BuildException(std::current_exception());
    }

    suspend_always initial_suspend() const noexcept
    {
        return {};
    }

    CYLazyFinalAwaiter final_suspend() const noexcept
    {
        return {};
    }

    template<class... ARGS_TYPES>
    void SetResult(ARGS_TYPES&&... args) noexcept(noexcept(type(std::forward<ARGS_TYPES>(args)...)))
    {
        m_objProducer.BuildResult(std::forward<ARGS_TYPES>(args)...);
    }

    type Get()
    {
        return m_objProducer.Get();
    }

private:
    CYProducerContext<type> m_objProducer;

};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_LAZY_RESULT_STATE_CORO_HPP__