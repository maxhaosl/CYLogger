#ifndef __CY_RESUME_ON_CORO_HPP__
#define __CY_RESUME_ON_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYExecutor.hpp"
#include "CYCoroutine/Results/Impl/CYConsumerContext.hpp"

#include <type_traits>

CYCOROUTINE_NAMESPACE_BEGIN

template<class EXECUTOR_TYPE>
class CYResumeOnAwaitable : public suspend_always
{
public:
    CYResumeOnAwaitable(EXECUTOR_TYPE& executor) noexcept
        : m_ptrExecutor(executor)
    {
    }

    void await_suspend(coroutine_handle<void> handle)
    {
        try
        {
            m_ptrExecutor.Post(CYAwaitViaFunctor{ handle, &m_bInterrupted });
        }
        catch (...)
        {
            // the exception caused the enqeueud CYTask to be broken and resumed with an interrupt, no need to do anything here.
        }
    }

    void await_resume() const
    {
        IfTrueThrow(m_bInterrupted, TEXT("CYResult - associated task was interrupted abnormally"));
    }

private:
    CYResumeOnAwaitable(CYResumeOnAwaitable&&) = delete;
    CYResumeOnAwaitable(const CYResumeOnAwaitable&) = delete;
    CYResumeOnAwaitable& operator=(CYResumeOnAwaitable&&) = delete;
    CYResumeOnAwaitable& operator=(const CYResumeOnAwaitable&) = delete;

private:
    EXECUTOR_TYPE& m_ptrExecutor;
    bool m_bInterrupted = false;
};

template<class EXECUTOR_TYPE>
auto ResumeOn(SharePtr<EXECUTOR_TYPE> ptrExecutor)
{
    static_assert(std::is_base_of_v<CYExecutor, EXECUTOR_TYPE>, "ResumeOn() - Given executor does not derive from executor");

    if (!static_cast<bool>(ptrExecutor))
    {
        throw std::invalid_argument("ResumeOn - Given executor is null.");
    }

    return CYResumeOnAwaitable<EXECUTOR_TYPE>(*ptrExecutor);
}

template<class EXECUTOR_TYPE>
auto ResumeOn(EXECUTOR_TYPE& ptrExecutor) noexcept
{
    return CYResumeOnAwaitable<EXECUTOR_TYPE>(ptrExecutor);
}
CYCOROUTINE_NAMESPACE_END

#endif //__CY_RESUME_ON_CORO_HPP__
