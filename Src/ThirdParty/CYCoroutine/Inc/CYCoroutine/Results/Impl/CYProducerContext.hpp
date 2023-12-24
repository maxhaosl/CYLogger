#ifndef __CY_PRODUCER_CONTEXT_HPPP__
#define __CY_PRODUCER_CONTEXT_HPPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

#include <cassert>
#include <exception>

CYCOROUTINE_NAMESPACE_BEGIN

template<class TYPE>
class CYProducerContext
{
public:
    virtual ~CYProducerContext() noexcept
    {
        switch (m_eStatus)
        {
        case EResultStatus::STATUS_RESULT_VALUE:
        {
            m_storage.object.~TYPE();
            break;
        }

        case EResultStatus::STATUS_RESULT_EXCEPTION:
        {
            m_storage.exception.~exception_ptr();
            break;
        }

        case EResultStatus::STATUS_RESULT_IDLE:
        {
            break;
        }

        default:
        {
            assert(false);
        }
        }
    }

    template<class... ARGS_TYPES>
    void BuildResult(ARGS_TYPES&&... args) noexcept(noexcept(TYPE(std::forward<ARGS_TYPES>(args)...)))
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        new (std::addressof(m_storage.object)) TYPE(std::forward<ARGS_TYPES>(args)...);
        m_eStatus = EResultStatus::STATUS_RESULT_VALUE;
    }

    void BuildException(const std::exception_ptr& exception) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        new (std::addressof(m_storage.exception)) std::exception_ptr(exception);
        m_eStatus = EResultStatus::STATUS_RESULT_EXCEPTION;
    }

    EResultStatus Status() const noexcept
    {
        return m_eStatus;
    }

    TYPE Get()
    {
        return std::move(GetRef());
    }

    TYPE& GetRef()
    {
        assert(m_eStatus != EResultStatus::STATUS_RESULT_IDLE);
        if (m_eStatus == EResultStatus::STATUS_RESULT_VALUE)
        {
            return m_storage.object;
        }

        assert(m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION);
        assert(static_cast<bool>(m_storage.exception));
        std::rethrow_exception(m_storage.exception);
    }

private:
    union CYStorage
    {
        TYPE object;
        std::exception_ptr exception;

        CYStorage() noexcept
        {
        }
        ~CYStorage() noexcept
        {
        }
    };

private:
    CYStorage m_storage;
    EResultStatus m_eStatus = EResultStatus::STATUS_RESULT_IDLE;
};

//////////////////////////////////////////////////////////////////////////
template<>
class CYProducerContext<void>
{
public:
    virtual ~CYProducerContext() noexcept
    {
        if (m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION)
        {
            m_storage.exception.~exception_ptr();
        }
    }

    CYProducerContext& operator=(CYProducerContext&& rhs) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        m_eStatus = std::exchange(rhs.m_eStatus, EResultStatus::STATUS_RESULT_IDLE);

        if (m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION)
        {
            new (std::addressof(m_storage.exception)) std::exception_ptr(rhs.m_storage.exception);
            rhs.m_storage.exception.~exception_ptr();
        }

        return *this;
    }

    void BuildResult() noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        m_eStatus = EResultStatus::STATUS_RESULT_VALUE;
    }

    void BuildException(const std::exception_ptr& except) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        new (std::addressof(m_storage.exception)) std::exception_ptr(except);
        m_eStatus = EResultStatus::STATUS_RESULT_EXCEPTION;
    }

    EResultStatus Status() const noexcept
    {
        return m_eStatus;
    }

    void Get() const
    {
        GetRef();
    }

    void GetRef() const
    {
        assert(m_eStatus != EResultStatus::STATUS_RESULT_IDLE);
        if (m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION)
        {
            assert(static_cast<bool>(m_storage.exception));
            std::rethrow_exception(m_storage.exception);
        }
    }

private:
    union CYStorage
    {
        std::exception_ptr exception;

        CYStorage() noexcept
        {
        }
        ~CYStorage() noexcept
        {
        }
    };

private:
    CYStorage m_storage;
    EResultStatus m_eStatus = EResultStatus::STATUS_RESULT_IDLE;

};

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
class CYProducerContext<TYPE&>
{
public:
    virtual ~CYProducerContext() noexcept
    {
        if (m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION)
        {
            m_storage.exception.~exception_ptr();
        }
    }

    CYProducerContext& operator=(CYProducerContext&& rhs) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        m_eStatus = std::exchange(rhs.m_eStatus, EResultStatus::STATUS_RESULT_IDLE);

        switch (m_eStatus)
        {
        case EResultStatus::STATUS_RESULT_VALUE:
        {
            m_storage.pointer = rhs.m_storage.pointer;
            break;
        }

        case EResultStatus::STATUS_RESULT_EXCEPTION:
        {
            new (std::addressof(m_storage.exception)) std::exception_ptr(rhs.m_storage.exception);
            rhs.m_storage.exception.~exception_ptr();
            break;
        }

        case EResultStatus::STATUS_RESULT_IDLE:
        {
            break;
        }

        default:
        {
            assert(false);
        }
        }

        return *this;
    }

    void BuildResult(TYPE& reference) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);

        auto pointer = std::addressof(reference);
        assert(pointer != nullptr);
        assert(reinterpret_cast<size_t>(pointer) % alignof(TYPE) == 0);

        m_storage.pointer = pointer;
        m_eStatus = EResultStatus::STATUS_RESULT_VALUE;
    }

    void BuildException(const std::exception_ptr& exception) noexcept
    {
        assert(m_eStatus == EResultStatus::STATUS_RESULT_IDLE);
        new (std::addressof(m_storage.exception)) std::exception_ptr(exception);
        m_eStatus = EResultStatus::STATUS_RESULT_EXCEPTION;
    }

    EResultStatus Status() const noexcept
    {
        return m_eStatus;
    }

    TYPE& Get() const
    {
        return GetRef();
    }

    TYPE& GetRef() const
    {
        assert(m_eStatus != EResultStatus::STATUS_RESULT_IDLE);

        if (m_eStatus == EResultStatus::STATUS_RESULT_VALUE)
        {
            assert(m_storage.pointer != nullptr);
            assert(reinterpret_cast<size_t>(m_storage.pointer) % alignof(TYPE) == 0);
            return *m_storage.pointer;
        }

        assert(m_eStatus == EResultStatus::STATUS_RESULT_EXCEPTION);
        assert(static_cast<bool>(m_storage.exception));
        std::rethrow_exception(m_storage.exception);
    }

private:
    union CYStorage
    {
        TYPE* pointer;
        std::exception_ptr exception;

        CYStorage() noexcept
        {
        }
        ~CYStorage() noexcept
        {
        }
    };

private:
    CYStorage m_storage;
    EResultStatus m_eStatus = EResultStatus::STATUS_RESULT_IDLE;
};

CYCOROUTINE_NAMESPACE_END


#endif //__CY_PRODUCER_CONTEXT_HPPP__