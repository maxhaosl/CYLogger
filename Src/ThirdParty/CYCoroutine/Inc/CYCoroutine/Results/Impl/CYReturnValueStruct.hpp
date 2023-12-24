#ifndef __CY_RETURN_VALUE_STRUCT_CORO_HPP__
#define __CY_RETURN_VALUE_STRUCT_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include <utility>

CYCOROUTINE_NAMESPACE_BEGIN

template<class DERIVED_TYPE, class TYPE>
struct CYReturnValueStruct
{
    template<class RETURN_TYPE>
    void return_value(RETURN_TYPE&& value)
    {
        auto self = static_cast<DERIVED_TYPE*>(this);
        self->SetResult(std::forward<RETURN_TYPE>(value));
    }
};

template<class DERIVED_TYPE>
struct CYReturnValueStruct<DERIVED_TYPE, void>
{
    void return_void() noexcept
    {
        auto self = static_cast<DERIVED_TYPE*>(this);
        self->SetResult();
    }
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_RETURN_VALUE_STRUCT_CORO_HPP__