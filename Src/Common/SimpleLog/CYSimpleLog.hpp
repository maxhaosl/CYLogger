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

#ifndef __CY_SIMPLE_LOG_HPP__
#define __CY_SIMPLE_LOG_HPP__

#include "Common/CYPrivateDefine.hpp"

#ifdef CYLOGGER_WIN_OS
#include <WinSock2.h>
#include <windows.h>
#else
#include <sys/time.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string>

  // console color definitions
enum ConsoleColor
{
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_YELLOW = 6,
    COLOR_WHITE = 7,
    COLOR_GRAY = 8,
    COLOR_INTENSITY = 0x08
};

CYLOGGER_NAMESPACE_BEGIN

// Constant definition
#define MAX_LOG_LINE_SIZE			8192
#define MAX_LOG_PATH_SIZE			1024
#define MAX_CONSOLE_TITLE_SIZE		128
#define DEFAULT_CONSOLE_TOTLE		TEXT("Log Info")

class CYSimpleLog
{
public:
    enum ESimpleLogType
    {
        LOG_TYPE_NONE = 0,
        LOG_TYPE_FILE,
        LOG_TYPE_CONSOLE,
        LOG_TYPE_ALL
    };

public:
    CYSimpleLog();
    ~CYSimpleLog();

    virtual bool InitLog(bool bLogTime = true, bool bLogLineCount = true, const TChar* szWorkPath = nullptr, const TChar* pszLogDir = TEXT("Log"), const TChar* szFilePath = nullptr, va_list args = nullptr) = 0;
    virtual bool WriteString(TChar* szStr) = 0;
    virtual void CloseLog() = 0;

    ESimpleLogType GetLogType() { return m_eLogType; }

    bool WriteLog(const TChar* szStr, ...);

protected:
    ESimpleLogType	m_eLogType;

    bool	m_bInit;
    bool	m_bLogTime;
    bool	m_bLogLineCount;
    int 	m_nLogLineCount;

    unsigned m_dwLineCount;
};

class CYSimpleLogFile : public CYSimpleLog
{
public:
    CYSimpleLogFile();
    ~CYSimpleLogFile();

    bool InitLog(bool bLogTime = true, bool bLogLineCount = true, const TChar* szWorkPath = nullptr, const TChar* pszLogDir = TEXT("Log"), const TChar* szFilePath = nullptr, va_list args = nullptr);
    void CloseLog();

    bool WriteString(TChar* szStr);

    void DeleteAllFile(TChar* szDir);

private:
    void CreateDir();

private:
    FILE* m_pFile;
    TChar	m_szLogDir[MAX_LOG_PATH_SIZE];
    TChar	m_szWorkPath[MAX_LOG_PATH_SIZE];
    TChar	m_szFilePath[MAX_LOG_PATH_SIZE];
};

class CYSimpleLogConsole : public CYSimpleLog
{
public:
    CYSimpleLogConsole();
    ~CYSimpleLogConsole();

    bool InitLog(bool bLogTime = true, bool bLogLineCount = true, const TChar* szWorkPath = nullptr, const TChar* pszLogDir = TEXT("Log"), const TChar* szConsoleTitle = nullptr, va_list args = nullptr);
    bool WriteString(TChar* szStr);
    void CloseLog();

    void Color(unsigned int color)
    {
#ifdef CYLOGGER_WIN_OS
        SetConsoleTextAttribute(m_hOutputHandle, (WORD)color);
#else
        // ANSI color codes for Unix-like systems
        const char* colors[] = {
            "\033[0;30m",  // BLACK
            "\033[0;34m",  // BLUE
            "\033[0;32m",  // GREEN
            "\033[0;36m",  // CYAN
            "\033[0;31m",  // RED
            "\033[0;35m",  // MAGENTA
            "\033[0;33m",  // YELLOW
            "\033[0;37m",  // WHITE
            "\033[1;30m",  // GRAY
            "\033[1m"      // INTENSITY
        };
        printf("%s", colors[color & 0x0F]);
#endif
    }

private:
#ifdef CYLOGGER_WIN_OS
    HANDLE	m_hInputHandle;
    HANDLE	m_hOutputHandle;
#else
    int     m_fdOut;
#endif
    TChar	m_szConsoleTitle[MAX_CONSOLE_TITLE_SIZE];
};

CYLOGGER_NAMESPACE_END

#endif	//__CY_SIMPLE_LOG_HPP__