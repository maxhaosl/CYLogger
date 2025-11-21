/*
 * CYLogger License
 * -----------
 *
 * CYLogger is licensed under the terms of the MIT license reproduced below.
 * This means that CYLogger is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 *
 *
 * ===============================================================================
 *
 * Copyright (C) 2023-2024 ShiLiang.Hao <newhaosl@163.com>, foobra<vipgs99@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ===============================================================================
 */

 /*
  * AUTHORS:  ShiLiang.Hao <newhaosl@163.com>, foobra<vipgs99@gmail.com>
  * VERSION:  1.0.0
  * PURPOSE:  A cross-platform efficient and stable Log library.
  * CREATION: 2023.04.15
  * LCHANGE:  2023.04.15
  * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
  */

#ifndef __CY_TIME_UTILS_HPP__
#define __CY_TIME_UTILS_HPP__

#include <iostream>
#include <cstdlib>
#include <stdint.h>

#ifdef CYLOGGER_WIN_OS
#include <windows.h>
#include <wincrypt.h>
//#include <intrin.h>
#else
#include <fcntl.h>
#include <sys/time.h>
#endif

#include "Inc/ICYLoggerDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

/// CPU operating cycle
static inline uint64_t rdtsc() noexcept
{
	// Read the hardware time stamp counter when available.
	// See https://en.wikipedia.org/wiki/Time_Stamp_Counter for more information.
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
	return __rdtsc();
#elif !defined(_MSC_VER) && defined(__i386__)
	uint64_t r = 0;
	__asm__ volatile ("rdtsc" : "=A"(r)); // Constrain the r variable to the eax:edx pair.
	return r;
#elif !defined(_MSC_VER) && (defined(__x86_64__) || defined(__amd64__))
	uint64_t r1 = 0, r2 = 0;
	__asm__ volatile ("rdtsc" : "=a"(r1), "=d"(r2)); // Constrain r1 to rax and r2 to rdx.
	return (r2 << 32) | r1;
#else
	// Fall back to using standard library clock (usually microsecond or nanosecond precision)
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#endif
}

CYLOGGER_NAMESPACE_END

#endif // __CY_TIME_UTILS_HPP__