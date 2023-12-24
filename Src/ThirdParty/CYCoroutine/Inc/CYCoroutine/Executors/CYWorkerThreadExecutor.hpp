#ifndef __CY_WORKER_THREAD_EXECUTOR_CORO_HPP__
#define __CY_WORKER_THREAD_EXECUTOR_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYDerivableExecutor.hpp"
#include "CYCoroutine/Threads/CYCacheLine.hpp"
#include "CYCoroutine/Threads/CYThread.hpp"

#include <deque>
#include <mutex>
#include <semaphore>

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API alignas(CACHE_LINE_ALIGNMENT) CYWorkerThreadExecutor final : public CYDerivableExecutor<CYWorkerThreadExecutor>
{
public:
    CYWorkerThreadExecutor(const FuncThreadDelegate & funStartedCallBack = {}, const FuncThreadDelegate & funTerminatedCallBack = {});

    void Enqueue(CYTask task) override;
    void Enqueue(std::span<CYTask> tasks) override;

    int  MaxConcurrencyLevel() const noexcept override;

    bool ShutdownRequested() const override;
    void ShutDown() override;

private:
    void MakeOSWorkerThread();
    bool DrainQueueImpl();
    bool DrainQueue();
    void WaitForTask(UniqueLock& lock);
    void WorkLoop();

    void EnqueueLocal(CYTask& task);
    void EnqueueLocal(std::span<CYTask> task);

    void EnqueueForeign(CYTask& task);
    void EnqueueForeign(std::span<CYTask> task);

private:
    bool             m_bPublicAbort;
    std::atomic_bool m_bPrivateAbort;
    alignas(CACHE_LINE_ALIGNMENT) std::mutex m_lock;

    std::deque<CYTask> m_lstPrivTaskQueue;
    std::deque<CYTask> m_lstPublicTaskQueue;

    CYThread m_thread;
    std::atomic_bool m_bAtomicAbort;
    std::binary_semaphore m_semaphore;

    const std::function<void(std::string_view)> m_funcStartedCallBack;
    const std::function<void(std::string_view)> m_funcTerminatedCallback;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_WORKER_THREAD_EXECUTOR_CORO_HPP__
