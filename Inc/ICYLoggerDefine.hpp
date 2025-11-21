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

#ifndef __I_CYLOGGER_DEFINE_HPP__
#define __I_CYLOGGER_DEFINE_HPP__

#include "CYCoroutine/CYTypeDefine.hpp"

#define CYLOGGER_NAMESPACE_BEGIN        namespace cry {
#define CYLOGGER_NAMESPACE              cry
#define CYLOGGER_NAMESPACE_END          }

#if defined(__MINGW32__)
#    define CYLOGGER_MINGW_OS
#elif defined(_WIN32)
#    define CYLOGGER_WIN_OS
#elif defined(unix) || defined(__unix__) || defined(__unix)
#    define CYLOGGER_UNIX_OS
#elif defined(__APPLE__) || defined(__MACH__)
#    define CYLOGGER_MAC_OS
#elif defined(__FreeBSD__)
#    define CYLOGGER_FREE_BSD_OS
#elif defined(__ANDROID__)
#    define CYLOGGER_ANDROID_OS
#elif defined(__linux__)
#    define CYLOGGER_LINUX_OS
#endif

#ifdef CYLOGGER_WIN_OS
#ifdef CYLOGGER_USE_DLL
#ifdef CYLOGGER_EXPORTS
#define CYLOGGER_API __declspec(dllexport)
#else
#define CYLOGGER_API __declspec(dllimport)
#endif
#else
#define CYLOGGER_API
#endif
#else
#define CYLOGGER_API __attribute__ ((visibility ("default")))
#endif

CYLOGGER_NAMESPACE_BEGIN

/**
* @brief The type of the logger.
*/
enum ELogType
{
    LOG_TYPE_NONE           = 0,
    LOG_TYPE_TRACE          = 1,                    ///< (Trace) type records detailed debugging information, such as method parameters and variable values.
    LOG_TYPE_DEBUG          = 2,                    ///< (Debug) type records some detailed information and pays more attention to the logical execution process of the code.
    LOG_TYPE_INFO           = 3,                    ///< (Info)  type records some important operating information, such as system startup, configuration loading and other operating status.
    LOG_TYPE_WARN           = 4,                    ///< (Warn)  type records some warning information. The program has some potential problems, but it will not cause the system to crash or affect the normal operation of the system.
    LOG_TYPE_ERROR          = 5,                    ///< (Error) type records some error information. The program has some abnormal situations that cannot be handled. Error level logs are usually accompanied by abnormal termination or unavailability of the system.
    LOG_TYPE_FATAL          = 6,                    ///< (Fatal) type records some very serious errors, indicating that the system can no longer run. Fatal level logs are usually accompanied by system crashes and data loss.
    LOG_TYPE_MAIN           = 7,                    ///< (Main)  Keep all logs.
    LOG_TYPE_REMOTE         = 8,                    ///< (Remote)Write to remote log.
    LOG_TYPE_SYS            = 9,                    ///< (Sys)   Write to system log.
    LOG_TYPE_MAX               ,
};

/**
* @brief The level of the message
*/
enum ELogLevel
{
    LOG_LEVEL_CONSOLE       = 1,
    LOG_LEVEL_TRACE         = 2,
    LOG_LEVEL_DEBUG         = 4,
    LOG_LEVEL_INFO          = 8,
    LOG_LEVEL_WARN          = 16,
    LOG_LEVEL_ERROR         = 32,
    LOG_LEVEL_FATAL         = 64,
    LOG_LEVEL_REMOTE        = 128,
    LOG_LEVEL_SYS           = 256
};

/**
* @brief Useful constants for specify level of logging - filter based on type of messages to be put in the Log
*/
enum ELogLevelFilter
{
    LOG_FILTER_ALL =
    LOG_LEVEL_CONSOLE |
    LOG_LEVEL_TRACE |
    LOG_LEVEL_DEBUG |
    LOG_LEVEL_INFO |
    LOG_LEVEL_WARN |
    LOG_LEVEL_ERROR  |
    LOG_LEVEL_FATAL,

    LOG_FILTER_WARNS_AND_ERRORS =
    LOG_LEVEL_INFO |
    LOG_LEVEL_WARN |
    LOG_LEVEL_ERROR |
    LOG_LEVEL_FATAL,

    LOG_FILTER_ERRORS =
    LOG_LEVEL_ERROR |
    LOG_LEVEL_FATAL,

    /**
    * Nothing will be put in the Log
    */
    LOG_FILTER_NONE = 0
};

/**
 * @brief Log file mode
*/
enum ELogFileMode
{
    LOG_MODE_FILE_APPEND            = 0x00,         ///< Append log file.
    LOG_MODE_FILE_TIME              = 0x01,         ///< Named log file by time.
};

/**
 * @brief Log Message Layout Type.
*/
enum ELogLayoutType
{
    LOG_LAYOUT_TYPE_CUSTOM          = 0x00,         ///< Log custom template.
    LOG_LAYOUT_TYPE_BUILDIN_1       = 0x01,         ///< Log built-in template 1.
    LOG_LAYOUT_TYPE_BUILDIN_2       = 0x02,         ///< Log built-in template 2.
    LOG_LAYOUT_TYPE_BUILDIN_3       = 0x03,         ///< Log built-in template 3.
};

/**
 * @brief Log statistics.
*/
struct STStatistics 
{
    uint64_t nTotalLine             = 0;             ///< The total number of logs written.
    uint64_t nTotalByte             = 0;             ///< Total log bytes written.
    double   fTotalCurrentFps       = 0;             ///< The current total fps written to the log.
    double   fTotalAverageFps       = 0;             ///< The average total fps written to the log.
    uint32_t nTotalPublicQueue      = 0;             ///< Total public queue length.
    uint32_t nTotalPrivateQueue     = 0;             ///< Total private queue length.

    uint64_t nConsoleLine           = 0;             ///< The total number of lines written to the console log.
    uint64_t nConsoleByte           = 0;             ///< Total bytes written to the console log.
    double   fConsoleCurrentFps     = 0;             ///< The current fps written to the console log.
    double   fConsoleAverageFps     = 0;             ///< The average fps written to the console log.
    uint32_t nConsolePublicDQueue   = 0;             ///< The console public debug queue length.
    uint32_t nConsolePublicTQueue   = 0;             ///< The console public trace queue length.
    uint32_t nConsolePublicIQueue   = 0;             ///< The console public info queue length.
    uint32_t nConsolePublicWQueue   = 0;             ///< The console public warn queue length.
    uint32_t nConsolePublicEQueue   = 0;             ///< The console public error queue length.
    uint32_t nConsolePublicFQueue   = 0;             ///< The console public fatal queue length.
    uint32_t nConsolePrivateQueue   = 0;             ///< The console private queue length.

    uint64_t nTraceLine             = 0;             ///< The total number of lines written to the trace log.
    uint64_t nTraceByte             = 0;             ///< Total bytes written to the trace log.
    double   fTraceCurrentFps       = 0;             ///< The current fps written to the trace log.
    double   fTraceAverageFps       = 0;             ///< The average fps written to the trace log.
    uint32_t nTracePublicQueue      = 0;             ///< The trace public queue length.
    uint32_t nTracePrivateQueue     = 0;             ///< The trace private queue length.

    uint64_t nDebugLine             = 0;             ///< The total number of lines written to the debug log.
    uint64_t nDebugByte             = 0;             ///< Total bytes written to the debug log.
    double   fDebugCurrentFps       = 0;             ///< The current fps written to the debug log.
    double   fDebugAverageFps       = 0;             ///< The average fps written to the debug log.
    uint32_t nDebugPublicQueue      = 0;             ///< The debug public queue length.
    uint32_t nDebugPrivateQueue     = 0;             ///< The debug private queue length.

    uint64_t nInfoLine              = 0;             ///< The total number of lines written to the info log.
    uint64_t nInfoByte              = 0;             ///< Total bytes written to the info log.
    double   fInfoCurrentFps        = 0;             ///< The current fps written to the info log.
    double   fInfoAverageFps        = 0;             ///< The average fps written to the info log.
    uint32_t nInfoPublicQueue       = 0;             ///< The info public queue length.
    uint32_t nInfoPrivateQueue      = 0;             ///< The info private queue length.

    uint64_t nWarnLine              = 0;             ///< The total number of lines written to the warn log.
    uint64_t nWarnByte              = 0;             ///< Total bytes written to the warn log.
    double   fWarnCurrentFps        = 0;             ///< The current fps written to the warn log.
    double   fWarnAverageFps        = 0;             ///< The average fps written to the warn log.
    uint32_t nWarnPublicQueue       = 0;             ///< The warn public queue length.
    uint32_t nWarnPrivateQueue      = 0;             ///< The warn private queue length.

    uint64_t nErrorLine             = 0;             ///< The total number of lines written to the error log.
    uint64_t nErrorByte             = 0;             ///< Total bytes written to the error log.
    double   fErrorCurrentFps       = 0;             ///< The current fps written to the error log.
    double   fErrorAverageFps       = 0;             ///< The average fps written to the error log.
    uint32_t nErrorPublicQueue      = 0;             ///< The error public queue length.
    uint32_t nErrorPrivateQueue     = 0;             ///< The error private queue length.

    uint64_t nFatalLine             = 0;             ///< The total number of lines written to the fatal log.
    uint64_t nFatalByte             = 0;             ///< Total bytes written to the fatal log.
    double   fFatalCurrentFps       = 0;             ///< The current fps written to the fatal log.
    double   fFatalAverageFps       = 0;             ///< The average fps written to the fatal log.
    uint32_t nFatalPublicQueue      = 0;             ///< The fatal public queue length.
    uint32_t nFatalPrivateQueue     = 0;             ///< The fatal private queue length.

    uint64_t nMainLine              = 0;             ///< The total number of lines written to the main log.
    uint64_t nMainByte              = 0;             ///< Total bytes written to the main log.
    double   fMainCurrentFps        = 0;             ///< The current fps written to the main log.
    double   fMainAverageFps        = 0;             ///< The average fps written to the main log.
    uint32_t nMainPublicDQueue      = 0;             ///< The main public debug queue length.
    uint32_t nMainPublicTQueue      = 0;             ///< The main public trace queue length.
    uint32_t nMainPublicIQueue      = 0;             ///< The main public info queue length.
    uint32_t nMainPublicWQueue      = 0;             ///< The main public warn queue length.
    uint32_t nMainPublicEQueue      = 0;             ///< The main public error queue length.
    uint32_t nMainPublicFQueue      = 0;             ///< The main public queue length.
    uint32_t nMainPrivateQueue      = 0;             ///< The main private queue length.

    uint64_t nRemoteLine            = 0;             ///< The total number of lines written to the remote log.
    uint64_t nRemoteByte            = 0;             ///< Total bytes written to the remote log.
    double   fRemoteCurrentFps      = 0;             ///< The current fps written to the remote log.
    double   fRemoteAverageFps      = 0;             ///< The average fps written to the remote log.
    uint32_t nRemotePublicDQueue    = 0;             ///< The remote public debug queue length.
    uint32_t nRemotePublicTQueue    = 0;             ///< The remote public trace queue length.
    uint32_t nRemotePublicIQueue    = 0;             ///< The remote public info queue length.
    uint32_t nRemotePublicWQueue    = 0;             ///< The remote public warn queue length.
    uint32_t nRemotePublicEQueue    = 0;             ///< The remote public error queue length.
    uint32_t nRemotePublicFQueue    = 0;             ///< The remote public  queue length.
    uint32_t nRemotePrivateQueue    = 0;             ///< The remote private queue length.

    uint64_t nSysLine               = 0;             ///< The total number of lines written to the system event log.
    uint64_t nSysByte               = 0;             ///< Total bytes written to the system event log.
    double   fSysCurrentFps         = 0;             ///< The current fps written to the system log.
    double   fSysAverageFps         = 0;             ///< The average fps written to the system log.
    uint32_t nSysPublicFQueue       = 0;             ///< The system public queue length.
    uint32_t nSysPrivateQueue       = 0;             ///< The system private queue length.
};

CYLOGGER_NAMESPACE_END

// CYLogger Config.
//////////////////////////////////////////////////////////////////////////
static constexpr bool                               LOG_SHOW_CONSOLE_WINDOW = false;                ///< Whether the console log displays the window.
static constexpr bool                               LOG_WRITE_REMOTE = false;                       ///< Whether to write logs to remote.
static constexpr bool                               LOG_WRITE_SYS = false;                          ///< Whether to write the error log to the system event log, only applicable to Windows operating system.
static constexpr CYLOGGER_NAMESPACE::ELogFileMode   LOG_FILE_MODE = CYLOGGER_NAMESPACE::ELogFileMode::LOG_MODE_FILE_TIME;    ///< Log file naming method.
static constexpr CYLOGGER_NAMESPACE::ELogLayoutType LOG_LAYOUT_TYPE = CYLOGGER_NAMESPACE::ELogLayoutType::LOG_LAYOUT_TYPE_BUILDIN_1; ///< Default log layout template.

// CYLogger Limit.
//////////////////////////////////////////////////////////////////////////
static constexpr bool                               LOG_LIMIT_ENABLE = true;                        ///< Detection switch is enabled by default.
static constexpr bool                               LOG_LIMIT_CLEAR_UNLOGFILE = true;               ///< Clean up non-log files in the log directory, such as package files and log files named in non-date format that remain after switching the date naming format for log files.

static constexpr int                                LOG_TIME_CLEAR_LOG = 60;                        ///< Time limit for cleaning expired log files, in seconds, default is 1 minutes.
static constexpr int                                LOG_TIME_EXPIRED_FILE = 24;                     ///< Limit for cleaning expired log files, in hours, default is 1 day.
static constexpr int                                LOG_CHECK_FILE_SIZE_TIME = 60 * 5;              ///< Detect the file size time interval, the second is the unit, the default is 5 minutes.
static constexpr int                                LOG_CHECK_FILE_COUNT_TIME = 60;                 ///< The time interval for detecting the number of log files, in seconds, the default is 1 minute.
static constexpr int                                LOG_CHECK_FILE_SIZE = 1024 * 1024 * 5;          ///< Detect file size limit, in bytes. If a single file exceeds this limit, it will automatically switch to a new file. This limit is only effective in the log file naming method based on date.
static constexpr int                                LOG_COUNT_PER_TYPE = 20;                        ///< The limit on the number of log files of each type. If there are multiple limits, which limit is triggered first and which limit is calculated according to.
static constexpr int                                LOG_CHECK_FILE_TYPE_SIZE = 1024 * 1024 * 500;   ///< The total log file size limit for each type of log. If this limit is exceeded, the log file with the earliest time and date will be cleared.
static constexpr int                                LOG_CHECK_FILE_ALL_SIZE = 1024 * 1024 * 1024;   ///< Limit the total size of the log file, in bytes, the default size is 1G.

// CYLogger Filter Level.
//////////////////////////////////////////////////////////////////////////
static constexpr CYLOGGER_NAMESPACE::ELogLevelFilter  LOG_LEVEL_FILTER = CYLOGGER_NAMESPACE::ELogLevelFilter::LOG_FILTER_ALL;  ///< Filter control log type writes.

#endif //__I_CYLOGGER_DEFINE_HPP__