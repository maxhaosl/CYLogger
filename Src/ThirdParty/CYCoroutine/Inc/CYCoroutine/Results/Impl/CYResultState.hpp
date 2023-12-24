#ifndef __CY_RESULT_STATE_CORO_HPP__
#define __CY_RESULT_STATE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/Impl/CYConsumerContext.hpp"
#include "CYCoroutine/Results/Impl/CYProducerContext.hpp"

#include <atomic>
#include <cassert>
#include <type_traits>

CYCOROUTINE_NAMESPACE_BEGIN

class CYCOROUTINE_API CYResultStateBase
{
public:
    enum class EResultState
    {
        STATE_RESULT_IDLE, STATE_RESULT_CONSUMER_SET, STATE_RESULT_CONSUMER_WAIT, STATE_RESULT_CONSUMER_DONE, STATE_RESULT_PRODUCER_DONE
    };

public:
    void Wait();
    bool await(coroutine_handle<void> handleCaller) noexcept;

    EResultState WhenAny(const SharePtr<CYWhenAnyContext>& ptrWhenAnyState) noexcept;

    void Share(const SharePtr<CYSharedResultStateBase>& resultState) noexcept;
    void TryRewindConsumer() noexcept;

protected:
    void AssertDone() const noexcept;

protected:
    std::atomic<EResultState> m_eResultState{ EResultState::STATE_RESULT_IDLE };
    CYConsumerContext m_objConsumer;
    coroutine_handle<void> m_handleDone;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYCOROUTINE_API CYResultState : public CYResultStateBase
{
public:
    template<class... ARGS_TYPES>
    void SetResult(ARGS_TYPES&&... args) noexcept(noexcept(TYPE(std::forward<ARGS_TYPES>(args)...)))
    {
        m_objProducer.BuildResult(std::forward<ARGS_TYPES>(args)...);
    }

    void SetException(const std::exception_ptr& error) noexcept
    {
        assert(error != nullptr);
        m_objProducer.BuildException(error);
    }

    // Consumer-side functions
    EResultStatus Status() const noexcept
    {
        const auto eState = m_eResultState.load(std::memory_order_acquire);
        assert(eState != EResultState::STATE_RESULT_CONSUMER_SET);

        if (eState == EResultState::STATE_RESULT_IDLE)
        {
            return EResultStatus::STATUS_RESULT_IDLE;
        }

        return m_objProducer.Status();
    }

    template<class DURATION_UNIT, class RATIO>
    EResultStatus WaitFor(std::chrono::duration<DURATION_UNIT, RATIO> nDuration)
    {
        const auto eState0 = m_eResultState.load(std::memory_order_acquire);
        if (eState0 == EResultState::STATE_RESULT_PRODUCER_DONE)
        {
            return m_objProducer.Status();
        }

        const auto objWaitCtx = MakeShared<std::binary_semaphore>(0);
        m_objConsumer.SetWaitForContext(objWaitCtx);

        std::atomic_thread_fence(std::memory_order_release);

        auto eExpectedIdleState = EResultState::STATE_RESULT_IDLE;
        const auto bIdle = m_eResultState.compare_exchange_strong(eExpectedIdleState, EResultState::STATE_RESULT_CONSUMER_SET, std::memory_order_acq_rel, std::memory_order_acquire);
        if (!bIdle)
        {
            AssertDone();
            return m_objProducer.Status();
        }

        if (objWaitCtx->try_acquire_for(nDuration + std::chrono::milliseconds(1)))
        {
            const auto status = m_eResultState.load(std::memory_order_acquire);
            (void)status;
            AssertDone();
            return m_objProducer.Status();
        }

        auto eExpectedConsumerState = EResultState::STATE_RESULT_CONSUMER_SET;
        const auto bIdle1 = m_eResultState.compare_exchange_strong(eExpectedConsumerState, EResultState::STATE_RESULT_IDLE, std::memory_order_acq_rel, std::memory_order_acquire);

        if (!bIdle1)
        {
            AssertDone();
            return m_objProducer.Status();
        }

        m_objConsumer.Clear();
        return EResultStatus::STATUS_RESULT_IDLE;
    }

    template<class CLOCK, class DURATION>
    EResultStatus WaitUntil(const std::chrono::time_point<CLOCK, DURATION>& timeoutTime)
    {
        const auto now = CLOCK::now();
        if (timeoutTime <= now)
        {
            return Status();
        }

        const auto diff = timeoutTime - now;
        return WaitFor(diff);
    }

    TYPE Get()
    {
        AssertDone();
        return m_objProducer.Get();
    }

    std::add_lvalue_reference_t<TYPE> GetRef()
    {
        AssertDone();
        return m_objProducer.GetRef();
    }

    template<class CALLABLE_TYPE>
    void FromCallable(CALLABLE_TYPE&& callable)
    {
        using is_void = std::is_same<TYPE, void>;

        try
        {
            FromCallable(is_void{}, std::forward<CALLABLE_TYPE>(callable));
        }
        catch (...)
        {
            SetException(std::current_exception());
        }
    }

    void CompleteProducer(coroutine_handle<void> handleDone = {})
    {
        m_handleDone = handleDone;

        const auto state_before = this->m_eResultState.exchange(EResultState::STATE_RESULT_PRODUCER_DONE, std::memory_order_acq_rel);
        assert(state_before != EResultState::STATE_RESULT_PRODUCER_DONE);

        switch (state_before)
        {
        case EResultState::STATE_RESULT_CONSUMER_SET:
        {
            return m_objConsumer.ResumeConsumer(*this);
        }

        case EResultState::STATE_RESULT_IDLE:
        {
            return;
        }

        case EResultState::STATE_RESULT_CONSUMER_WAIT:
        {
            return m_eResultState.notify_one();
        }

        case EResultState::STATE_RESULT_CONSUMER_DONE:
        {
            return DeleteSelf(this);
        }

        default:
        {
            break;
        }
        }

        assert(false);
    }

    void CompleteConsumer() noexcept
    {
        const auto EResultState = this->m_eResultState.load(std::memory_order_acquire);
        if (EResultState == EResultState::STATE_RESULT_PRODUCER_DONE)
        {
            return DeleteSelf(this);
        }

        const auto eState = this->m_eResultState.exchange(EResultState::STATE_RESULT_CONSUMER_DONE, std::memory_order_acq_rel);
        assert(eState != EResultState::STATE_RESULT_CONSUMER_SET);

        if (eState == EResultState::STATE_RESULT_PRODUCER_DONE)
        {
            return DeleteSelf(this);
        }

        assert(eState == EResultState::STATE_RESULT_IDLE);
    }

    void CompleteJoinedConsumer() noexcept
    {
        AssertDone();
        DeleteSelf(this);
    }

private:
    static void DeleteSelf(CYResultState<TYPE>* pState) noexcept
    {
        auto handleDone = pState->m_handleDone;
        if (static_cast<bool>(handleDone))
        {
            assert(handleDone.done());
            return handleDone.destroy();
        }

#ifdef CYCOROUTINE_GCC_COMPILER
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif
        delete pState;
#ifdef CYCOROUTINE_GCC_COMPILER
#    pragma GCC diagnostic pop
#endif
    }


    template<class CALLABLE_TYPE>
    void FromCallable(std::true_type /*is_void_type*/, CALLABLE_TYPE&& callable)
    {
        callable();
        SetResult();
    }

    template<class CALLABLE_TYPE>
    void FromCallable(std::false_type /*is_void_type*/, CALLABLE_TYPE&& callable)
    {
        SetResult(callable());
    }

private:
    CYProducerContext<TYPE> m_objProducer;

};

template<class TYPE>
struct ConsumerResultStateDeleter
{
    void operator()(CYResultState<TYPE>* pState) const noexcept
    {
        assert(pState != nullptr);
        pState->CompleteConsumer();
    }
};

template<class TYPE>
struct JoinedConsumerResultStateDeleter
{
    void operator()(CYResultState<TYPE>* pState) const noexcept
    {
        assert(pState != nullptr);
        pState->CompleteJoinedConsumer();
    }
};

template<class TYPE>
struct ProducerResultStateDeleter
{
    void operator()(CYResultState<TYPE>* pState) const
    {
        assert(pState != nullptr);
        pState->CompleteProducer();
    }
};

template<class TYPE>
using CYConsumerResultStatePtr = UniquePtr<CYResultState<TYPE>, ConsumerResultStateDeleter<TYPE>>;

template<class TYPE>
using CYJoinedConsumerResultStatePtr = UniquePtr<CYResultState<TYPE>, JoinedConsumerResultStateDeleter<TYPE>>;

template<class TYPE>
using CYProducerResultStatePtr = UniquePtr<CYResultState<TYPE>, ProducerResultStateDeleter<TYPE>>;


CYCOROUTINE_NAMESPACE_END

#endif //__CY_RESULT_STATE_CORO_HPP__
