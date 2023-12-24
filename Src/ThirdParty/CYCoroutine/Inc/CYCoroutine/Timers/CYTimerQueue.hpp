#ifndef __CY_TIMER_QUEUE_CORO_HPP__
#define __CY_TIMER_QUEUE_CORO_HPP__

#include "CYCoroutine/Common/CYBind.hpp"
#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "CYCoroutine/Results/CYLazyResult.hpp"
#include "CYCoroutine/Threads/CYThread.hpp"
#include "CYTimer.hpp"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include <cassert>

CYCOROUTINE_NAMESPACE_BEGIN

enum class ETimerRequest
{
    TYPE_TIMER_REQUEST_ADD,
    TYPE_TIMER_REQUEST_REMOVE
};

using TimerPtr = SharePtr<CYTimerStateBase>;
using RequestQueue = std::vector<std::pair<TimerPtr, ETimerRequest>>;
using milliseconds = std::chrono::milliseconds;

class CYCOROUTINE_API CYTimerQueue : public std::enable_shared_from_this<CYTimerQueue>
{
    friend class CYTimer;
public:
    CYTimerQueue(milliseconds nMaxWaitTime, const FuncThreadDelegate& funThreadStartedCallback = {}, const FuncThreadDelegate& funThreadTerminatedCallback = {});
    ~CYTimerQueue() noexcept;

public:
    void ShutDown();
    bool ShutdownRequested() const noexcept;

    milliseconds MaxWorkerIdleTime() const noexcept;
    CYLazyResult<void> MakeDelayObject(milliseconds nDueTime, SharePtr<CYExecutor> ptrExecutor);

public:
    template<class CALLABLE_TYPE, class... ARGS_TYPES>
    CYTimer MakeTimer(milliseconds nDueTime, milliseconds nFrequency, SharePtr<CYExecutor> ptrExecutor, CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
    {
        if (!static_cast<bool>(ptrExecutor))
        {
            throw std::invalid_argument("CYTimerQueue::MakeTimer() - CYExecutor is null.");
        }

        return MakeTimerImpl(nDueTime.count(), nFrequency.count(), std::move(ptrExecutor), false, Bind(std::forward<CALLABLE_TYPE>(callable), std::forward<ARGS_TYPES>(args)...));
    }

    template<class CALLABLE_TYPE, class... ARGS_TYPES>
    CYTimer MakeOneShotTimer(milliseconds nDueTime, SharePtr<CYExecutor> ptrExecutor, CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
    {
        if (!static_cast<bool>(ptrExecutor))
        {
            throw std::invalid_argument("CYTimerQueue::MakeOneShotTimer() - CYExecutor is null.");
        }

        return MakeTimerImpl(nDueTime.count(), 0, std::move(ptrExecutor), true, Bind(std::forward<CALLABLE_TYPE>(callable), std::forward<ARGS_TYPES>(args)...));
    }

private:
    CYThread EnsureWorkerThread(UniqueLock& lock);

    void AddInternalTimer(UniqueLock& lock, TimerPtr ptrNewTimer);
    void RemoveInternalTimer(TimerPtr ptrExistTimer);

    void AddTimer(UniqueLock& lock, TimerPtr ptrNewTimer);

    CYLazyResult<void> MakeDelayObjectImpl(milliseconds nDueTime, SharePtr<CYTimerQueue> ptrSelf, SharePtr<CYExecutor> ptrExecutor);

    template<class CALLABLE_TYPE>
    TimerPtr MakeTimerImpl(size_t nDueTime, size_t nFrequency, SharePtr<CYExecutor> ptrExecutor, bool isOneShot, CALLABLE_TYPE&& callable)
    {
        assert(static_cast<bool>(ptrExecutor));

        using DecayedType = typename std::decay_t<CALLABLE_TYPE>;
        auto timerState = MakeShared<CYTimerState<DecayedType>>(nDueTime, nFrequency, std::move(ptrExecutor), weak_from_this(), isOneShot, std::forward<CALLABLE_TYPE>(callable));
        {
            UniqueLock lock(m_lock);
            AddTimer(lock, timerState);
        }

        return timerState;
    }

    void WorkLoop();

private:
    std::atomic_bool m_bAtomicAbort;
    std::mutex m_lock;
    RequestQueue m_lstRequestQueue;
    CYThread m_objWorkerThread;
    std::condition_variable m_condition;
    bool m_bAbort;
    bool m_bIdle;
    const milliseconds m_objMaxWaitingTime;
    const FuncThreadDelegate m_funcStartedCallBack;
    const FuncThreadDelegate m_funcTerminatedCallback;

};
CYCOROUTINE_NAMESPACE_END

#endif  // __CY_TIMER_QUEUE_CORO_HPP__
