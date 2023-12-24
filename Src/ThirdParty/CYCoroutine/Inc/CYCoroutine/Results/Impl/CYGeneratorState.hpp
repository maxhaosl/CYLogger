#ifndef __CY_GENERATOR_STATE_CORO_HPP__
#define __CY_GENERATOR_STATE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<typename type>
class CYGenerator;

template<typename TYPE>
class CYGeneratorState
{
public:
    using value_type = std::remove_reference_t<TYPE>;

public:
    CYGenerator<TYPE> get_return_object() noexcept
    {
        return CYGenerator<TYPE> {coroutine_handle<CYGeneratorState<TYPE>>::from_promise(*this)};
    }

    suspend_always initial_suspend() const noexcept
    {
        return {};
    }

    suspend_always final_suspend() const noexcept
    {
        return {};
    }

    suspend_always yield_value(value_type& ref) noexcept
    {
        m_pValue = std::addressof(ref);
        return {};
    }

    suspend_always yield_value(value_type&& ref) noexcept
    {
        m_pValue = std::addressof(ref);
        return {};
    }

    void unhandled_exception() noexcept
    {
        m_pException = std::current_exception();
    }

    void return_void() const noexcept
    {
    }

    value_type& value() const noexcept
    {
        assert(m_pValue != nullptr);
        assert(reinterpret_cast<std::intptr_t>(m_pValue) % alignof(value_type) == 0);
        return *m_pValue;
    }

    void throw_if_exception() const
    {
        if (static_cast<bool>(m_pException))
        {
            std::rethrow_exception(m_pException);
        }
    }

private:
    value_type* m_pValue = nullptr;
    std::exception_ptr m_pException;
};

//////////////////////////////////////////////////////////////////////////
struct CYGeneratorEndIterator
{
};

//////////////////////////////////////////////////////////////////////////
template<typename TYPE>
class CYGeneratorIterator
{
public:
    using value_type = std::remove_reference_t<TYPE>;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;

public:
    CYGeneratorIterator(coroutine_handle<CYGeneratorState<TYPE>> handle) noexcept
        : m_handleCoro(handle)
    {
        assert(static_cast<bool>(m_handleCoro));
    }

    CYGeneratorIterator& operator++()
    {
        assert(static_cast<bool>(m_handleCoro));
        assert(!m_handleCoro.done());
        m_handleCoro.resume();

        if (m_handleCoro.done())
        {
            m_handleCoro.promise().throw_if_exception();
        }

        return *this;
    }

    void operator++(int)
    {
        (void)operator++();
    }

    reference operator*() const noexcept
    {
        assert(static_cast<bool>(m_handleCoro));
        return m_handleCoro.promise().value();
    }

    pointer operator->() const noexcept
    {
        assert(static_cast<bool>(m_handleCoro));
        return std::addressof(operator*());
    }

    friend bool operator==(const CYGeneratorIterator& it0, const CYGeneratorIterator& it1) noexcept
    {
        return it0.m_handleCoro == it1.m_handleCoro;
    }

    friend bool operator==(const CYGeneratorIterator& it, CYGeneratorEndIterator) noexcept
    {
        return it.m_handleCoro.done();
    }

    friend bool operator==(CYGeneratorEndIterator end_it, const CYGeneratorIterator& it) noexcept
    {
        return (it == end_it);
    }

    friend bool operator!=(const CYGeneratorIterator& it, CYGeneratorEndIterator end_it) noexcept
    {
        return !(it == end_it);
    }

    friend bool operator!=(CYGeneratorEndIterator end_it, const CYGeneratorIterator& it) noexcept
    {
        return it != end_it;
    }

private:
    coroutine_handle<CYGeneratorState<TYPE>> m_handleCoro;

};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_GENERATOR_STATE_CORO_HPP__