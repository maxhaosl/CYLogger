#ifndef __CY_PROMISES_CORO_HPP__
#define __CY_PROMISES_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "CYCoroutine/Results/Impl/CYLazyResultState.hpp"
#include "CYCoroutine/Results/Impl/CYResultState.hpp"
#include "CYCoroutine/Results/Impl/CYReturnValueStruct.hpp"
#include "CYCoroutine/Task/CYTask.hpp"

#include <vector>

CYCOROUTINE_NAMESPACE_BEGIN

class CYExecutor;

template<class EXECUTOR_TYPE>
class CYRescheduledPromise
{
    static_assert(std::is_base_of_v<CYExecutor, EXECUTOR_TYPE>, "<<EXECUTOR_TYPE>> isn't driven from CYExecutor.");
public:
    template<class... ARGS_TYPES>
    CYRescheduledPromise(CYExecutorTag, EXECUTOR_TYPE* pExecutor, ARGS_TYPES&&...)
        : m_initialExecutor(ToRef(pExecutor))
    {
    }

    template<class... ARGS_TYPES>
    CYRescheduledPromise(CYExecutorTag, EXECUTOR_TYPE& pExecutor, ARGS_TYPES&&...)
        : m_initialExecutor(pExecutor)
    {
    }

    template<class... ARGS_TYPES>
    CYRescheduledPromise(CYExecutorTag, SharePtr<EXECUTOR_TYPE> CYExecutor, ARGS_TYPES&&... ARGS)
        : CYRescheduledPromise(CYExecutorTag{}, CYExecutor.get(), std::forward<ARGS_TYPES>(ARGS)...)
    {
    }

    template<class class_type, class... ARGS_TYPES>
    CYRescheduledPromise(class_type&&, CYExecutorTag, SharePtr<EXECUTOR_TYPE> CYExecutor, ARGS_TYPES&&... ARGS)
        : CYRescheduledPromise(CYExecutorTag{}, * CYExecutor, std::forward<ARGS_TYPES>(ARGS)...)
    {
    }

    //////////////////////////////////////////////////////////////////////////
    class CYSchedulingAwaiter : public suspend_always
    {

    private:
        bool m_bInterrupted = false;

    public:
        template<class promise_type>
        void await_suspend(coroutine_handle<promise_type> handle)
        {
            try
            {
                handle.promise().m_initialExecutor.Post(CYAwaitViaFunctor{ handle, &m_bInterrupted });
            }
            catch (...)
            {
                // do nothing. ~CYAwaitViaFunctor will resume the coroutine and throw an exception.
            }
        }

        void await_resume() const
        {
            if (m_bInterrupted)
            {
                IfTrueThrow(m_bInterrupted, TEXT("CYResult - associated task was interrupted abnormally"));
            }
        }
    };

    CYSchedulingAwaiter initial_suspend() const noexcept
    {
        return {};
    }

private:
    EXECUTOR_TYPE& m_initialExecutor;

    static EXECUTOR_TYPE& ToRef(EXECUTOR_TYPE* pExecutor)
    {
        if (pExecutor == nullptr)
        {
            throw std::invalid_argument("parallel-coroutine - given executor is null.");
        }

        return *pExecutor;
    }
};

//////////////////////////////////////////////////////////////////////////
struct CYResumedPromise
{
    suspend_never initial_suspend() const noexcept
    {
        return {};
    }
};

//////////////////////////////////////////////////////////////////////////
struct CYNuLLResultPromise
{
    CYNuLLResult get_return_object() const noexcept
    {
        return {};
    }

    suspend_never final_suspend() const noexcept
    {
        return {};
    }

    void unhandled_exception() const noexcept
    {
    }
    void return_void() const noexcept
    {
    }
};

//////////////////////////////////////////////////////////////////////////
struct CYResultPublisher : public suspend_always
{
    template<class promise_type>
    void await_suspend(coroutine_handle<promise_type> handle) const noexcept
    {
        handle.promise().CompleteProducer(handle);
    }
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYResult;

template<class TYPE>
struct CYResultCoroPromise : public CYReturnValueStruct<CYResultCoroPromise<TYPE>, TYPE>
{

private:
    CYResultState<TYPE> m_objResultState;

public:
    template<class... ARGS_TYPES>
    void SetResult(ARGS_TYPES&&... ARGS) noexcept(noexcept(TYPE(std::forward<ARGS_TYPES>(ARGS)...)))
    {
        this->m_objResultState.SetResult(std::forward<ARGS_TYPES>(ARGS)...);
    }

    void unhandled_exception() noexcept
    {
        this->m_objResultState.SetException(std::current_exception());
    }

    CYResult<TYPE> get_return_object() noexcept
    {
        return { &m_objResultState };
    }

    void CompleteProducer(coroutine_handle<void> done_handle) noexcept
    {
        this->m_objResultState.CompleteProducer(done_handle);
    }

    CYResultPublisher final_suspend() const noexcept
    {
        return {};
    }
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
struct CYLazyPromise : CYLazyResultState<TYPE>, public CYReturnValueStruct<CYLazyPromise<TYPE>, TYPE>
{
};

//////////////////////////////////////////////////////////////////////////
struct CYResumedNuLLResultPromise : public CYResumedPromise, public CYNuLLResultPromise
{
};

//////////////////////////////////////////////////////////////////////////
template<class RETURN_TYPE>
struct CYResumedResultPromise : public CYResumedPromise, public CYResultCoroPromise<RETURN_TYPE>
{
};

//////////////////////////////////////////////////////////////////////////
template<class EXECUTOR_TYPE>
struct CYRescheduledNuLLResultPromise : public CYRescheduledPromise<EXECUTOR_TYPE>, public CYNuLLResultPromise
{
    using CYRescheduledPromise<EXECUTOR_TYPE>::CYRescheduledPromise;
};

//////////////////////////////////////////////////////////////////////////
template<class RETURN_TYPE, class EXECUTOR_TYPE>
struct CYRescheduledResultPromise : public CYRescheduledPromise<EXECUTOR_TYPE>, public CYResultCoroPromise<RETURN_TYPE>
{
    using CYRescheduledPromise<EXECUTOR_TYPE>::CYRescheduledPromise;
};

CYCOROUTINE_NAMESPACE_END

namespace COROUTINE_NAMESPACE_STD
{
    // No CYExecutor + No CYResult
    template<class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYNuLLResult, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYResumedNuLLResultPromise;
    };

    // No CYExecutor + CYResult
    template<class TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYResult<TYPE>, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYResumedResultPromise<TYPE>;
    };

    // Executor + no CYResult
    template<class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYNuLLResult, CYCOROUTINE_NAMESPACE::CYExecutorTag, SharePtr<EXECUTOR_TYPE>, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledNuLLResultPromise<EXECUTOR_TYPE>;
    };

    template<class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYNuLLResult, CYCOROUTINE_NAMESPACE::CYExecutorTag, EXECUTOR_TYPE*, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledNuLLResultPromise<EXECUTOR_TYPE>;
    };

    template<class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYNuLLResult, CYCOROUTINE_NAMESPACE::CYExecutorTag, EXECUTOR_TYPE&, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledNuLLResultPromise<EXECUTOR_TYPE>;
    };

    // Executor + CYResult
    template<class TYPE, class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYResult<TYPE>, CYCOROUTINE_NAMESPACE::CYExecutorTag, SharePtr<EXECUTOR_TYPE>, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledResultPromise<TYPE, EXECUTOR_TYPE>;
    };

    template<class TYPE, class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYResult<TYPE>, CYCOROUTINE_NAMESPACE::CYExecutorTag, EXECUTOR_TYPE*, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledResultPromise<TYPE, EXECUTOR_TYPE>;
    };

    template<class TYPE, class EXECUTOR_TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYResult<TYPE>, CYCOROUTINE_NAMESPACE::CYExecutorTag, EXECUTOR_TYPE&, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYRescheduledResultPromise<TYPE, EXECUTOR_TYPE>;
    };

    // Lazy
    template<class TYPE, class... ARGS>
    struct coroutine_traits<CYCOROUTINE_NAMESPACE::CYLazyResult<TYPE>, ARGS...>
    {
        using promise_type = CYCOROUTINE_NAMESPACE::CYLazyPromise<TYPE>;
    };

}  // namespace COROUTINE_NAMESPACE_STD

#endif // __CY_PROMISES_CORO_HPP__
