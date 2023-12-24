#ifndef __CY_MAKE_RESULT_CORO_HPP__
#define __CY_MAKE_RESULT_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Results/CYResult.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
template<class TYPE, class... ARGS_TYPES>
CYResult<TYPE> MakeReadyResult(ARGS_TYPES&&... args)
{
    static_assert(std::is_constructible_v<TYPE, ARGS_TYPES...> || std::is_same_v<TYPE, void>, "MakeReadyResult - <<TYPE>> is not constructible from <<ARGS_TYPES...>");
    static_assert(std::is_same_v<TYPE, void> ? (sizeof...(ARGS_TYPES) == 0) : true, "MakeReadyResult<void> - this overload does not accept any argument.");

    CYProducerResultStatePtr<TYPE> ptrPromise(new CYResultState<TYPE>());
    CYConsumerResultStatePtr<TYPE> ptrState(ptrPromise.get());

    ptrPromise->SetResult(std::forward<ARGS_TYPES>(args)...);
    ptrPromise.reset();  // publish the result;

    return { std::move(ptrState) };
}

//////////////////////////////////////////////////////////////////////////
template<class TYPE>
CYResult<TYPE> MakeExceptionalResult(std::exception_ptr pException)
{
    if (!static_cast<bool>(pException))
    {
        throw std::invalid_argument("make_exception_result() - given exception_ptr is null.");
    }

    CYProducerResultStatePtr<TYPE> ptrPromise(new CYResultState<TYPE>());
    CYConsumerResultStatePtr<TYPE> ptrState(ptrPromise.get());

    ptrPromise->SetException(pException);
    ptrPromise.reset();  // publish the result;

    return { std::move(ptrState) };
}

//////////////////////////////////////////////////////////////////////////
template<class TYPE, class EXCEPTION_TYPE>
CYResult<TYPE> MakeExceptionalResult(EXCEPTION_TYPE objException)
{
    return MakeExceptionalResult<TYPE>(std::make_exception_ptr(objException));
}
CYCOROUTINE_NAMESPACE_END

#endif //__CY_MAKE_RESULT_CORO_HPP__