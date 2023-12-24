#ifndef __CY_COROUTINE_ENGINE_DEFINE_CORO_HPP__
#define __CY_COROUTINE_ENGINE_DEFINE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include <cstddef>

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
class CYExecutor;
class CYCOROUTINE_API ICYExecutorCollection
{
public:
    ICYExecutorCollection() noexcept = default;
    virtual ~ICYExecutorCollection() noexcept = default;

public:
    virtual void RegisterExecutor(SharePtr<CYExecutor> ptrExecutor) = 0;
    virtual void ShutDownALL() = 0;
};

//////////////////////////////////////////////////////////////////////////
using milliseconds = std::chrono::milliseconds;
struct CYCOROUTINE_API CYCoroutineOptions
{
public:
    CYCoroutineOptions() noexcept;

    CYCoroutineOptions(const CYCoroutineOptions&) noexcept = default;
    CYCoroutineOptions& operator=(const CYCoroutineOptions&) noexcept = default;

public:
    size_t maxCpuThreads;
    milliseconds maxThreadPoolExecutorWaitTime;

    size_t maxBackgroundThreads;
    milliseconds maxBackgroundExecutorWaitTime;
    milliseconds maxTimerQueueWaitTime;

    FuncThreadDelegate funStartedCallBack;
    FuncThreadDelegate funTerminatedCallBack;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_COROUTINE_ENGINE_DEFINE_CORO_HPP__
