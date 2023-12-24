#ifndef __CY_GENERATOR_CORO_HPP__
#define __CY_GENERATOR_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/Impl/CYGeneratorState.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<typename TYPE>
class CYGenerator
{
public:
    using promise_type = CYGeneratorState<TYPE>;
    using iterator = CYGeneratorIterator<TYPE>;
    static_assert(!std::is_same_v<TYPE, void>, "<<TYPE>> can not be void.");

public:
    CYGenerator(coroutine_handle<promise_type> handle) noexcept
        : m_handleCoro(handle)
    {
    }

    CYGenerator(CYGenerator&& rhs) noexcept
        : m_handleCoro(std::exchange(rhs.m_handleCoro, {}))
    {
    }

    virtual ~CYGenerator() noexcept
    {
        if (static_cast<bool>(m_handleCoro))
        {
            m_handleCoro.destroy();
        }
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_handleCoro);
    }

    iterator begin()
    {
        IfTrueThrow(!static_cast<bool>(m_handleCoro), TEXT("CYGenerator::begin - CYGenerator is empty."));

        assert(!m_handleCoro.done());
        m_handleCoro.resume();

        if (m_handleCoro.done())
        {
            m_handleCoro.promise().throw_if_exception();
        }

        return iterator{ m_handleCoro };
    }

    static CYGeneratorEndIterator end() noexcept
    {
        return {};
    }

private:
    CYGenerator(const CYGenerator& rhs) = delete;
    CYGenerator& operator=(CYGenerator&& rhs) = delete;
    CYGenerator& operator=(const CYGenerator& rhs) = delete;

private:
    coroutine_handle<promise_type> m_handleCoro;
};
CYCOROUTINE_NAMESPACE_END

#endif //__CY_GENERATOR_CORO_HPP__