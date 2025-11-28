#ifndef __CY_JTHREAD_DEFINE_HPP__
#define __CY_JTHREAD_DEFINE_HPP__

#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#if defined(_WIN32)
#include <thread>
#include <stop_token>

#else
#include <thread>
#include <atomic>
#include <functional>
#include <utility>
#endif

#ifdef _WIN32
using cy_jthread = std::jthread;
#else
using cy_jthread = std::thread;
#endif

#endif // __CY_JTHREAD_DEFINE_HPP__