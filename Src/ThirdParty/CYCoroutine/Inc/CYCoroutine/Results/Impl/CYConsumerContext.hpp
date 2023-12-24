#ifndef __CY_CONSUMER_CONTEXT_CORO_HPP__
#define __CY_CONSUMER_CONTEXT_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

#include <atomic>
#include <semaphore>

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API CYAwaitViaFunctor
{
public:
    CYAwaitViaFunctor(coroutine_handle<void> handleCaller, bool* pbInterrupted) noexcept;
    CYAwaitViaFunctor(CYAwaitViaFunctor&& rhs) noexcept;
    ~CYAwaitViaFunctor() noexcept;

    void operator()() noexcept;

private:
    bool* m_pbInterrupted;
    coroutine_handle<void> m_handleCaller;
};

//////////////////////////////////////////////////////////////////////////
class CYResultStateBase;
class CYCOROUTINE_API CYWhenAnyContext
{
public:
    CYWhenAnyContext(coroutine_handle<void> handleCoro) noexcept;

public:
    bool AnyResultFinished() const noexcept;
    bool FinishProcessing() noexcept;
    const CYResultStateBase* CompletedResult() const noexcept;

    void TryResume(CYResultStateBase& CompletedResult) noexcept;
    bool ResumeInline(CYResultStateBase& CompletedResult) noexcept;

private:
    std::atomic<const CYResultStateBase*> m_status;
    coroutine_handle<void> m_coro_handle;

    static const CYResultStateBase* k_pProcessing;
    static const CYResultStateBase* k_pDoneProcessing;
};

//////////////////////////////////////////////////////////////////////////
class CYSharedResultStateBase;
class CYCOROUTINE_API CYConsumerContext
{

public:
    virtual ~CYConsumerContext() noexcept;

    void Clear() noexcept;
    void ResumeConsumer(CYResultStateBase& self) const;

    void SetAwaitHandle(coroutine_handle<void> handleCaller) noexcept;
    void SetWaitForContext(const SharePtr<std::binary_semaphore>& ptrWaitCtx) noexcept;
    void SetWhenAnyContext(const SharePtr<CYWhenAnyContext>& ptrWhenAnyCtx) noexcept;
    void SetSharedContext(const SharePtr<CYSharedResultStateBase>& ptrSharedCtx) noexcept;

private:
    void Destroy() noexcept;

private:
    enum class EConsumerStatus
    {
        STATUS_CONSUMER_IDLE, STATUS_CONSUMER_AWAIT, STATUS_CONSUMER_WAITFOR, STATUS_CONSUMER_WHENANY, STATUS_CONSUMER_SHARED
    };

    union CYStorage
    {
        coroutine_handle<void> handleCaller;
        SharePtr<std::binary_semaphore> wait_for_ctx;
        SharePtr<CYWhenAnyContext> ptrWhenAnyCtx;
        WeakPtr<CYSharedResultStateBase> ptrSharedCtx;

        CYStorage() noexcept
        {
        }
        ~CYStorage() noexcept
        {
        }
    };

private:
    EConsumerStatus m_status = EConsumerStatus::STATUS_CONSUMER_IDLE;
    CYStorage m_storage;
};

CYCOROUTINE_NAMESPACE_END


#endif //__CY_CONSUMER_CONTEXT_CORO_HPP__
