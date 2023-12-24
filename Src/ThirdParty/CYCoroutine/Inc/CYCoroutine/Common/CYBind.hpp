#ifndef __CY_BIND_CORO_HPP__
#define __CY_BIND_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include <tuple>
#include <type_traits>

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE>
auto&& Bind(CALLABLE_TYPE&& callable)
{
    return std::forward<CALLABLE_TYPE>(callable);
}

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE, class... ARGS_TYPES>
auto Bind(CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
{
    constexpr static auto inti = std::is_nothrow_invocable_v<CALLABLE_TYPE, ARGS_TYPES...>;
    return [callable = std::forward<CALLABLE_TYPE>(callable), tuple = MakeTuple(std::forward<ARGS_TYPES>(args)...)]() mutable noexcept(inti) -> decltype(auto)
        {
            return std::apply(callable, tuple);
        };
}

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE>
auto&& BindWithTryCatchImpl(std::true_type /*is_noexcept*/, CALLABLE_TYPE&& callable)
{
    return std::forward<CALLABLE_TYPE>(callable);
}

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE>
auto BindWithTryCatchImpl(std::false_type /*is_noexcept*/, CALLABLE_TYPE&& callable)
{
    return [callable = std::forward<CALLABLE_TYPE>(callable)]() mutable noexcept {
        try
        {
            callable();
        }
        catch (...)
        {
            // do nothing
        }
        };  // no args to Bind
}

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE>
auto BindWithTryCatch(CALLABLE_TYPE&& callable)
{
    using isNoexcept = typename std::is_nothrow_invocable<CALLABLE_TYPE>::type;
    return BindWithTryCatchImpl(isNoexcept{}, std::forward<CALLABLE_TYPE>(callable));
}

//////////////////////////////////////////////////////////////////////////
template<class CALLABLE_TYPE, class... ARGS_TYPES>
auto BindWithTryCatch(CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
{
    return BindWithTryCatch(Bind(std::forward<CALLABLE_TYPE>(callable), std::forward<ARGS_TYPES>(args)...));
}

CYCOROUTINE_NAMESPACE_END

#endif //__CY_BIND_CORO_HPP__