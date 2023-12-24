#ifndef __CY_ASYNC_CONDITION_CORO_HPP__
#define __CY_ASYNC_CONDITION_CORO_HPP__

#include "CYCoroutine/Common/CYList.hpp"
#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/CYLazyResult.hpp"
#include "CYCoroutine/Threads/CYAsyncLock.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
class CYAsyncCondition;
class CYCOROUTINE_API CYAWaiter
{
public:
    CYAWaiter(CYAsyncCondition& parent, CScopedAsyncLock& lock) noexcept;
    virtual ~CYAWaiter() noexcept = default;

public:
    constexpr bool  await_ready() const noexcept;
    void            await_suspend(coroutine_handle<void> handleCaller);
    void            await_resume() const noexcept;
    void            resume() noexcept;

public:
    CYAWaiter*        next = nullptr;

private:
    CYAsyncCondition& m_parent;
    CScopedAsyncLock& m_lock;
    coroutine_handle<void> m_handleCaller;
};

//////////////////////////////////////////////////////////////////////////
class CYCOROUTINE_API CYAsyncCondition
{
    friend CYAWaiter;
public:
    CYAsyncCondition() noexcept = default;
    ~CYAsyncCondition() noexcept;

public:
    CYLazyResult<void> await(SharePtr<CYExecutor> ptrResumeExecutor, CScopedAsyncLock& lock);

    template<class PREDICATE_TYPE>
    CYLazyResult<void> await(SharePtr<CYExecutor> ptrResumeExecutor, CScopedAsyncLock& lock, PREDICATE_TYPE pred)
    {
        static_assert(std::is_invocable_r_v<bool, PREDICATE_TYPE>, "Given predicate isn't invocable with no args, or does not return a type which is or convertible to bool.");

        VerifyAwaitParams(ptrResumeExecutor, lock);
        return AWaitImpl(std::move(ptrResumeExecutor), lock, pred);
    }

    void NotifyOne();
    void NotifyALL();

private:
    CYAsyncCondition(const CYAsyncCondition&) noexcept = delete;
    CYAsyncCondition(CYAsyncCondition&&) noexcept = delete;

    CYLazyResult<void> AWaitImpl(SharePtr<CYExecutor> ptrResumeExecutor, CScopedAsyncLock& lock);

    template<class PREDICATE_TYPE>
    CYLazyResult<void> AWaitImpl(SharePtr<CYExecutor> ptrResumeExecutor, CScopedAsyncLock& lock, PREDICATE_TYPE pred)
    {
        while (true)
        {
            assert(lock.OwnsLock());
            if (pred())
            {
                break;
            }

            co_await AWaitImpl(ptrResumeExecutor, lock);
        }
    }

    static void VerifyAwaitParams(const SharePtr<CYExecutor>& ptrResumeExecutor, const CScopedAsyncLock& lock);

private:
    std::mutex          m_lock;
    CYList<CYAWaiter>   m_awaiters;

};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_ASYNC_CONDITION_CORO_HPP__