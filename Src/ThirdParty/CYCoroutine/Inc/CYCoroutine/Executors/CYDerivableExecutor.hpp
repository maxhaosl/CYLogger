#ifndef __CY_DERIVABLE_EXECUTOR_CORO_HPP__
#define __CY_DERIVABLE_EXECUTOR_CORO_HPP__

#include "CYCoroutine/Common/CYBind.hpp"
#include "CYCoroutine/CYCoroutineDefine.hpp"
#include "CYCoroutine/Executors/CYExecutor.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

template<class CONCRETE_EXECUTOR_TYPE>
struct CYCOROUTINE_API CYDerivableExecutor : public CYExecutor
{
    CYDerivableExecutor(std::string_view strName)
        : CYExecutor(strName)
    {
    }

    template<class CALLABLE_TYPE, class... ARGS_TYPES>
    void Post(CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
    {
        return DoPost<CONCRETE_EXECUTOR_TYPE>(std::forward<CALLABLE_TYPE>(callable), std::forward<ARGS_TYPES>(args)...);
    }

    template<class CALLABLE_TYPE, class... ARGS_TYPES>
    auto Submit(CALLABLE_TYPE&& callable, ARGS_TYPES&&... args)
    {
        return DoSubmit<CONCRETE_EXECUTOR_TYPE>(std::forward<CALLABLE_TYPE>(callable), std::forward<ARGS_TYPES>(args)...);
    }

    template<class CALLABLE_TYPE>
    void BulkPost(std::span<CALLABLE_TYPE> lstCallable)
    {
        return DoBulkPost<CONCRETE_EXECUTOR_TYPE>(lstCallable);
    }

    template<class CALLABLE_TYPE, class RETURN_TYPE = std::invoke_result_t<CALLABLE_TYPE>>
    std::vector<CYResult<RETURN_TYPE>> BulkSubmit(std::span<CALLABLE_TYPE> lstCallable)
    {
        return DoBulkSubmit<CONCRETE_EXECUTOR_TYPE>(lstCallable);
    }
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_DERIVABLE_EXECUTOR_CORO_HPP__
