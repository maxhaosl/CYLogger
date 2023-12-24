#ifndef __CY_SHARED_RESULT_AWAITTABLE_CORO_HPP__
#define __CY_SHARED_RESULT_AWAITTABLE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/Impl/CYSharedResultState.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<class TYPE>
class CYSharedAwaitableBase : public suspend_always
{
public:
    CYSharedAwaitableBase(const SharePtr<CYSharedResultState<TYPE>>& ptrState) noexcept
        : m_ptrState(ptrState)
    {
    }

protected:
    SharePtr<CYSharedResultState<TYPE>> m_ptrState;

private:
    CYSharedAwaitableBase(const CYSharedAwaitableBase&) = delete;
    CYSharedAwaitableBase(CYSharedAwaitableBase&&) = delete;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYSharedAwaitable : public CYSharedAwaitableBase<TYPE>
{
public:
    CYSharedAwaitable(const SharePtr<CYSharedResultState<TYPE>>& ptrState) noexcept
        : CYSharedAwaitableBase<TYPE>(ptrState)
    {
    }

    bool await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        assert(static_cast<bool>(this->m_ptrState));
        this->m_objAwaitCtx.handleCaller = handleCaller;
        return this->m_ptrState->await(m_objAwaitCtx);
    }

    std::add_lvalue_reference_t<TYPE> await_resume()
    {
        return this->m_ptrState->Get();
    }

private:
    CYSharedAwaitContext m_objAwaitCtx;
};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYSharedResult;

template<class TYPE>
class CYSharedResolveAwaitable : public CYSharedAwaitableBase<TYPE>
{
public:
    CYSharedResolveAwaitable(const SharePtr<CYSharedResultState<TYPE>>& ptrState) noexcept :
        CYSharedAwaitableBase<TYPE>(ptrState)
    {
    }

    bool await_suspend(coroutine_handle<void> handleCaller) noexcept
    {
        assert(static_cast<bool>(this->m_ptrState));
        this->m_objAwaitCtx.handleCaller = handleCaller;
        return this->m_ptrState->await(m_objAwaitCtx);
    }

    CYSharedResult<TYPE> await_resume()
    {
        return CYSharedResult<TYPE>(std::move(this->m_ptrState));
    }

private:
    CYSharedAwaitContext m_objAwaitCtx;
};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_SHARED_RESULT_AWAITTABLE_CORO_HPP__