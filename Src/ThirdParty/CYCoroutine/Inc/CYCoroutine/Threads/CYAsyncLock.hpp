#ifndef __CY_ASYNC_LOCK_CORO_HPP__
#define __CY_ASYNC_LOCK_CORO_HPP__

#include "CYCoroutine/Common/CYList.hpp"
#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYExecutor.hpp"
#include "CYCoroutine/Results/CYLazyResult.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API CAsyncLockAwaiter
{
    friend class CAsyncLock;
public:
    inline CAsyncLockAwaiter(CAsyncLock& parent, UniqueLock& lock) noexcept;

    constexpr bool  await_ready() const noexcept;
    void            await_suspend(coroutine_handle<void> hHandle);
    constexpr void  await_resume() const noexcept;
    void            retry() noexcept;

public:
    CAsyncLockAwaiter*      next = nullptr;

private:
    CAsyncLock&             m_parent;
    UniqueLock              m_lock;
    coroutine_handle<void>  m_handleResume;
};

//////////////////////////////////////////////////////////////////////////
class CScopedAsyncLock;
class CYCOROUTINE_API CAsyncLock
{
    friend class CScopedAsyncLock;
    friend class CAsyncLockAwaiter;
public:
    virtual ~CAsyncLock() noexcept;

    CYLazyResult<CScopedAsyncLock>  Lock(SharePtr<CYExecutor> ptrResumeExecutor);
    CYLazyResult<bool>              TryLock();
    void                            UnLock();

private:
    CYLazyResult<CScopedAsyncLock>  LockImpl(SharePtr<CYExecutor> ptrResumeExecutor, bool bWithRAIIGuard);

private:
    bool                        m_bLocked = false;
    std::mutex                  m_mutexAwaiter;
    CYList<CAsyncLockAwaiter>   m_lstAwaiters;

#ifdef CYCOROUTINE_DEBUG_MODE
    std::atomic_intptr_t        m_nThreadCountInCriticalSection{ 0 };
#endif
};

//////////////////////////////////////////////////////////////////////////
class CYCOROUTINE_API CScopedAsyncLock
{
public:
    inline CScopedAsyncLock() noexcept = default;
    inline CScopedAsyncLock(CScopedAsyncLock&& rhs) noexcept;

    inline CScopedAsyncLock(CAsyncLock& lock, std::defer_lock_t) noexcept;
    inline CScopedAsyncLock(CAsyncLock& lock, std::adopt_lock_t) noexcept;

    virtual ~CScopedAsyncLock() noexcept;

public:
    CYLazyResult<void>  Lock(SharePtr<CYExecutor> ptrResumeExecutor);
    CYLazyResult<bool>  TryLock();
    void                UnLock();

    bool                OwnsLock() const noexcept;
    explicit operator bool() const noexcept;

    void                Swap(CScopedAsyncLock& rhs) noexcept;
    CAsyncLock* Release() noexcept;
    CAsyncLock* Mutex() const noexcept;

private:
    CAsyncLock* m_pLock = nullptr;
    bool                m_bOwns = false;

};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_ASYNC_LOCK_CORO_HPP__
