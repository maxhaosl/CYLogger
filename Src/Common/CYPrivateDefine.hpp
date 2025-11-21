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

#ifndef __CY_PRIVATE_DEFINE_HPP__
#define __CY_PRIVATE_DEFINE_HPP__

#include "Inc/ICYLoggerDefine.hpp"
#include "CYCoroutine/CYTypeDefine.hpp"

#ifdef CYLOGGER_WIN_OS
#include <winsock2.h>
#include <windows.h>
#include <sys/timeb.h>
#include <tchar.h>
#else
#include <sys/time.h>
#include <chrono>
#include <unistd.h>
#include <string.h>
#endif

CYLOGGER_NAMESPACE_BEGIN

#ifdef CYLOGGER_WIN_OS
#define TRED		FOREGROUND_RED | FOREGROUND_INTENSITY
#define TGREEN		FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TYELLOW		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define TNORMAL		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#else
// Unix/Linux color codes
#define TRED		"\033[1;31m"
#define TGREEN		"\033[1;32m"
#define TYELLOW		"\033[1;33m"
#define TNORMAL		"\033[0m"
#endif
#define TWHITE		TNORMAL | FOREGROUND_INTENSITY
#define TBLUE		FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY

#define LOG_DIR  TEXT("Log")

#ifdef CYLOGGER_WIN_OS
#define LOG_SEPARATOR		TEXT('\\')
#define LOG_SEPARATOR_STR	TEXT("\\")
#else
#define LOG_SEPARATOR		'/'
#define LOG_SEPARATOR_STR	"/"
#endif

/**
 * @brief Return Code.
*/
enum RetCode : int16_t
{
    ERR_NOERR = 0x00,
    ERR_COND_TIMEOUT = 0x01,
};

CYLOGGER_NAMESPACE_END

#endif //__CY_PRIVATE_DEFINE_HPP__