#ifndef __CY_COROUTINE_ENGINE_CORO_HPP__
#define __CY_COROUTINE_ENGINE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Engine/CYCoroutineEngineDefine.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
class CYTimerQueue;
class CYManualExecutor;
class CYInlineExecutor;
class CYThreadPoolExecutor;
class CYThreadExecutor;
class CYWorkerThreadExecutor;
class CYCOROUTINE_API CYCoroutineEngine
{
public:
    CYCoroutineEngine();
    CYCoroutineEngine(const CYCoroutineOptions& options);

    virtual ~CYCoroutineEngine() noexcept;

public:
    SharePtr<CYTimerQueue>          TimerQueue() const noexcept;

    //////////////////////////////////////////////////////////////////////////
    SharePtr<CYInlineExecutor>      InlineExecutor() const noexcept;
    SharePtr<CYThreadPoolExecutor>  ThreadPoolExecutor() const noexcept;
    SharePtr<CYThreadPoolExecutor>  BackgroundExecutor() const noexcept;
    SharePtr<CYThreadExecutor>      ThreadExecutor() const noexcept;

    //////////////////////////////////////////////////////////////////////////
    SharePtr<CYWorkerThreadExecutor> MakeWorkerThreadExecutor();
    SharePtr<CYManualExecutor> MakeManualExecutor();


    template<class EXECUTOR_TYPE, class... ARGS_TYPES>
    SharePtr<EXECUTOR_TYPE> MakeExecutor(ARGS_TYPES&&... args)
    {
        static_assert(std::is_base_of_v<CYExecutor, EXECUTOR_TYPE>, "<<EXECUTOR_TYPE>> is not a derived class of CYExecutor.");
        static_assert(std::is_constructible_v<EXECUTOR_TYPE, ARGS_TYPES...>, "Can not Build <<EXECUTOR_TYPE>> from <<ARGS_TYPES...>>.");
        static_assert(!std::is_abstract_v<EXECUTOR_TYPE>, "<<EXECUTOR_TYPE>> is an abstract class.");

        auto ptrExecutor = MakeShared<EXECUTOR_TYPE>(std::forward<ARGS_TYPES>(args)...);
        m_ptrRegisteredExecutors->RegisterExecutor(ptrExecutor);
        return ptrExecutor;
    }

    // Singleton.
    //////////////////////////////////////////////////////////////////////////
    static SharePtr<CYCoroutineEngine> GetInstance();
    static void                        FreeInstance();
    static std::tuple<unsigned int, unsigned int, unsigned int> Version() noexcept;
private:
    CYCoroutineOptions                      m_objEngineOptions;
    mutable SharePtr<ICYExecutorCollection> m_ptrRegisteredExecutors;

    mutable SharePtr<CYTimerQueue>          m_ptrTimerQueue;
    mutable SharePtr<CYInlineExecutor>      m_ptrInlineExecutor;
    mutable SharePtr<CYThreadExecutor>      m_ptrThreadExecutor;
    mutable SharePtr<CYThreadPoolExecutor>  m_ptrThreadPoolExecutor;
    mutable SharePtr<CYThreadPoolExecutor>  m_ptrBackgroundExecutor;
    static SharePtr<CYCoroutineEngine>      m_ptrInstance;
};

CYCOROUTINE_NAMESPACE_END

#define CYInlineCoro()      CYCOROUTINE_NAMESPACE::CYCoroutineEngine::GetInstance()->InlineExecutor()
#define CYThreadPoolCoro()  CYCOROUTINE_NAMESPACE::CYCoroutineEngine::GetInstance()->ThreadPoolExecutor()
#define CYBackgroundCoro()  CYCOROUTINE_NAMESPACE::CYCoroutineEngine::GetInstance()->BackgroundExecutor()
#define CYThreadCoro()      CYCOROUTINE_NAMESPACE::CYCoroutineEngine::GetInstance()->ThreadExecutor()
#define CYCoroFree()        CYCOROUTINE_NAMESPACE::CYCoroutineEngine::FreeInstance()
#endif //__CY_COROUTINE_ENGINE_CORO_HPP__
