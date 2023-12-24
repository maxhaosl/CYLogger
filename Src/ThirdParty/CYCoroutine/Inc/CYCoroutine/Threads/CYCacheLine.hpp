#ifndef __CY_CACHE_LINE_CORO_HPP__
#define __CY_CACHE_LINE_CORO_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

#include <new>

#if !defined(CYCOROUTINE_MAC_OS) && defined(__cpp_lib_hardware_interference_size)
#    if defined(CYCOROUTINE_GCC_COMPILER) 
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Winterference-size"
#    endif
constexpr inline std::size_t CACHE_LINE_ALIGNMENT = std::hardware_destructive_interference_size;
#    if defined(CYCOROUTINE_GCC_COMPILER) 
#        pragma GCC diagnostic pop
#    endif
#else
constexpr inline std::size_t CACHE_LINE_ALIGNMENT = 64;
#endif

#endif //__CY_CACHE_LINE_CORO_HPP__
