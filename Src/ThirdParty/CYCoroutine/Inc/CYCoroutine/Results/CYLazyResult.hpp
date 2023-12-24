#ifndef __CY_LAZY_RESULT_CORO_HPP__
#define __CY_LAZY_RESULT_CORO_HPP__

#include "CYCoroutine/Results/CYLazyResultAwaitable.hpp"
#include "CYCoroutine/Results/CYPromises.hpp"
#include "CYCoroutine/Results/Impl/CYLazyResultState.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<class TYPE>
class CYLazyResult
{
public:
    CYLazyResult() noexcept = default;

    CYLazyResult(CYLazyResult&& rhs) noexcept
        : m_handleState(std::exchange(rhs.m_handleState, {}))
    {
    }

    CYLazyResult(coroutine_handle<CYLazyResultState<TYPE>> handleState) noexcept
        : m_handleState(handleState)
    {
    }

    virtual ~CYLazyResult() noexcept
    {
        if (static_cast<bool>(m_handleState))
        {
            m_handleState.destroy();
        }
    }

    CYLazyResult& operator=(CYLazyResult&& rhs) noexcept
    {
        if (&rhs == this)
        {
            return *this;
        }

        if (static_cast<bool>(m_handleState))
        {
            m_handleState.destroy();
        }

        m_handleState = std::exchange(rhs.m_handleState, {});
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_handleState);
    }

    EResultStatus Status() const
    {
        IfEmptyThrow("CYLazyResult::status - result is empty.");
        return m_handleState.promise().Status();
    }

    auto operator co_await()
    {
        IfEmptyThrow("CYLazyResult::operator co_await - result is empty.");
        return CYLazyAwaitable<TYPE> {std::exchange(m_handleState, {})};
    }

    auto Resolve()
    {
        IfEmptyThrow("CYLazyResult::resolve - result is empty.");
        return CYLazyResolveAwaitable<TYPE> {std::exchange(m_handleState, {})};
    }

    CYResult<TYPE> Run()
    {
        IfEmptyThrow("CYLazyResult::run - result is empty.");
        return RunImpl();
    }

private:
    void IfEmptyThrow(const char* pErrMsg) const
    {
        IfTrueThrow(!static_cast<bool>(m_handleState), AtoT(pErrMsg));
    }

    CYResult<TYPE> RunImpl()
    {
        CYLazyResult self(std::move(*this));
        co_return co_await self;
    }

private:
    coroutine_handle<CYLazyResultState<TYPE>> m_handleState;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_LAZY_RESULT_CORO_HPP__