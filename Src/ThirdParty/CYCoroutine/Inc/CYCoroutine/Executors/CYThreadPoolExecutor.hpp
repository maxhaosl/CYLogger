#ifndef __CY_THREAD_POOL_EXECUTOR_CORO_HPP__
#define __CY_THREAD_POOL_EXECUTOR_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYDerivableExecutor.hpp"
#include "CYCoroutine/Threads/CYCacheLine.hpp"
#include "CYCoroutine/Threads/CYThread.hpp"

#include <deque>
#include <mutex>

CYCOROUTINE_NAMESPACE_BEGIN

class CYIdleWorkerSet
{
    enum class EIdlStatus
    {
        STATUS_IDLE_IDLE, STATUS_IDLE_ACTIVE
    };

    struct alignas(CACHE_LINE_ALIGNMENT) CYPaddedFlag
    {
        std::atomic<EIdlStatus> eFlag{ EIdlStatus::STATUS_IDLE_ACTIVE };
    };

public:
    CYIdleWorkerSet(size_t size);
    virtual ~CYIdleWorkerSet() noexcept = default;

    void SetIdle(size_t nIdleThread) noexcept;
    void SetActive(size_t nIdleThread) noexcept;

    size_t FindIdleWorker(size_t nCallerIndex) noexcept;
    void FindIdleWorkers(size_t nCallerIndex, std::vector<size_t>& lstResultBuffer, size_t nMaxCount) noexcept;

private:
    bool TryAcquireFlag(size_t index) noexcept;

private:
    std::atomic_intptr_t m_nApproxSize;
    const UniquePtr<CYPaddedFlag[]> m_ptrIdleFlags;
    const size_t m_nSize;

};

//////////////////////////////////////////////////////////////////////////
class CYThreadPoolWorker;
class CYCOROUTINE_API alignas(CACHE_LINE_ALIGNMENT) CYThreadPoolExecutor final
    : public CYDerivableExecutor<CYThreadPoolExecutor>
{
    friend class CYThreadPoolWorker;
 public:
    CYThreadPoolExecutor(std::string_view strPoolName, size_t nPoolSize, std::chrono::milliseconds maxIdleTime, const FuncThreadDelegate& funStartedCallBack = {}, const FuncThreadDelegate& funTerminatedCallBack = {});
    virtual ~CYThreadPoolExecutor() override;

    void Enqueue(CYTask task) override;
    void Enqueue(std::span<CYTask> tasks) override;

    int  MaxConcurrencyLevel() const noexcept override;

    bool ShutdownRequested() const override;
    void ShutDown() override;

    std::chrono::milliseconds MaxWorkerIdleTime() const noexcept;

private:
    void MarkWorkerIdle(size_t index) noexcept;
    void MarkWorkerActive(size_t index) noexcept;
    void FindIdleWorkers(size_t nCallerIndex, std::vector<size_t>& buffer, size_t nMaxCount) noexcept;

    CYThreadPoolWorker& WorkerAt(size_t index) noexcept;

private:
    std::vector<CYThreadPoolWorker> m_lstWorkers;
    alignas(CACHE_LINE_ALIGNMENT) CYIdleWorkerSet m_objIdleWorkers;
    alignas(CACHE_LINE_ALIGNMENT) std::atomic_size_t m_nRoundRobinCursor;
    alignas(CACHE_LINE_ALIGNMENT) std::atomic_bool m_bAbort;

};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_THREAD_POOL_EXECUTOR_CORO_HPP__
