#ifndef __CY_THREAD_EXECUTOR_CORO_HPP__
#define __CY_THREAD_EXECUTOR_CORO_HPP__

#include "CYCoroutine/Executors/CYDerivableExecutor.hpp"
#include "CYCoroutine/Threads/CYCacheLine.hpp"
#include "CYCoroutine/Threads/CYThread.hpp"

#include <condition_variable>
#include <list>
#include <mutex>
#include <span>

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API alignas(CACHE_LINE_ALIGNMENT) CYThreadExecutor final : public CYDerivableExecutor<CYThreadExecutor>
{
public:
    CYThreadExecutor(const FuncThreadDelegate & funStartedCallBack = {}, const FuncThreadDelegate & funTerminatedCallBack = {});
    ~CYThreadExecutor() noexcept;

    void Enqueue(CYTask task) override;
    void Enqueue(std::span<CYTask> tasks) override;

    int MaxConcurrencyLevel() const noexcept override;

    bool ShutdownRequested() const override;
    void ShutDown() override;

private:
    void EnqueueImpl(UniqueLock& lock, CYTask& task);
    void RetireWorker(std::list<CYThread>::iterator it);

private:
    bool m_bAbort;
    std::mutex m_lock;
    std::list<CYThread> m_lstWorkers;
    std::condition_variable m_condition;
    std::list<CYThread> m_lstLastRetired;
    std::atomic_bool m_bAtomicAbort;
    const FuncThreadDelegate m_funcStartedCallBack;
    const FuncThreadDelegate m_funcTerminatedCallback;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_THREAD_EXECUTOR_CORO_HPP__
