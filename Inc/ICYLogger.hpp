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

/*
 * UPDATES LOG
 *
 * VERSION 1.0.0 <DATE: 2023.04.15>
 *     create the first project.
 *     support Log level, support one day one Log file.
 *     support multi-thread, cross-platform.
 *
 *
 */

#ifndef __I_CYLOGGER_HPP__
#define __I_CYLOGGER_HPP__

#include "ICYLoggerDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

class ICYLoggerPatternFilter;
class ICYLoggerTemplateLayout;
class CYLOGGER_API ICYLogger
{
public:
    ICYLogger()
    {
    }
    virtual ~ICYLogger()
    {
    }

public:
    /**
     * @brief Initialization and de-initialization of cry Log.
    */
    virtual bool Init() = 0;
    virtual void UnInit() = 0;

    /**
     * @brief Flush Log.
    */
    virtual void Flush(ELogType eLogType = ELogType::LOG_TYPE_MAX) = 0;

    /**
     * @brief Append Log file.
    */
    virtual bool AddApender(ELogType eLogType, const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode) = 0;

    /**
     * @brief Write Log.
    */
    virtual void WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* szMsg) = 0;
    virtual void WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) = 0;
    virtual void WriteEscapeLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) = 0;
    virtual void WriteHexLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const void* szMsg, int nLen) = 0;

    /**
     * @brief Set Log Config.
    */
    virtual void SetConfig(const TChar* szLogPath, bool bShowConsoleWindow) = 0;

    /**
     * @brief Set restriction rules
    */
    virtual void SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize) = 0;

    /**
     * @brief Log special character filtering.
     * @param pFilter needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetFilter(ICYLoggerPatternFilter* pFilter) = 0;

    /**
     * @brief Log information format template.
     * @param pLayout needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetLayout(ELogLayoutType eLayoutType, ICYLoggerTemplateLayout* pLayout) = 0;

    /**
     * @brief Get statistics.
    */
    virtual bool GetStats(STStatistics* pStats) = 0;

public:
    /**
     * @brief Get Instance.
    */
    static ICYLogger* GetInstance(ELogLevelFilter eLevelFilter);

    /**
     * @brief Free Instance.
    */
    static void FreeInstance();
};

CYLOGGER_NAMESPACE_END

//////////////////////////////////////////////////////////////////////////
#if LOG_WRITE_REMOTE
#define LOG_LEVEL_REMOTE_TAIL      CYLOGGER_NAMESPACE::LOG_LEVEL_REMOTE
#else
#define LOG_LEVEL_REMOTE_TAIL      0  
#endif

#if LOG_WRITE_SYS
#define LOG_LEVEL_SYS_TAIL          CYLOGGER_NAMESPACE::LOG_LEVEL_SYS
#else
#define LOG_LEVEL_SYS_TAIL          0  
#endif

#define LOG_LEVEL_TAIL  LOG_LEVEL_REMOTE_TAIL | LOG_LEVEL_SYS_TAIL

//////////////////////////////////////////////////////////////////////////
#define CY_LOG()                            CYLOGGER_NAMESPACE::ICYLogger::GetInstance(LOG_LEVEL_FILTER)
#define CY_LOG_FREE()                       CYLOGGER_NAMESPACE::ICYLogger::FreeInstance()

#define CY_LOG_APPENDER()                   CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_TRACE, nullptr, TEXT("Trace.log"),        LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_DEBUG, nullptr, TEXT("Debug.log"),        LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_INFO,  nullptr, TEXT("Info.log"),         LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_WARN,  nullptr, TEXT("Warning.log"),      LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_ERROR, nullptr, TEXT("Error.log"),        LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_FATAL, nullptr, TEXT("Fatal.log"),        LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_MAIN,  nullptr, TEXT("Main.log"),         LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_REMOTE,nullptr, TEXT("127.0.0.1:7000"),   LOG_FILE_MODE);     \
                                            CY_LOG()->AddApender(CYLOGGER_NAMESPACE::LOG_TYPE_SYS,   nullptr, TEXT("CYLogger"),         LOG_FILE_MODE); 

#define CY_LOG_CONFIG(strPath, bShowConsoleWindow, eLogLayoutType)  \
                                            CY_LOG()->SetConfig(strPath, bShowConsoleWindow);  \
                                            CY_LOG()->SetLayout(eLogLayoutType, nullptr); \
                                            CY_LOG_APPENDER() \
                                            CY_LOG()->SetRestriction(LOG_LIMIT_ENABLE, LOG_LIMIT_CLEAR_UNLOGFILE, LOG_TIME_CLEAR_LOG, LOG_TIME_EXPIRED_FILE, LOG_CHECK_FILE_SIZE_TIME, LOG_CHECK_FILE_COUNT_TIME, LOG_CHECK_FILE_SIZE, LOG_COUNT_PER_TYPE, LOG_CHECK_FILE_TYPE_SIZE, LOG_CHECK_FILE_ALL_SIZE);  

//////////////////////////////////////////////////////////////////////////
#define CY_LOG_ESCAPE_TRACE(szMsg, ...)     CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,   CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ESCAPE_DEBUG(szMsg, ...)     CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_DEBUG | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,   CYLOGGER_NAMESPACE::LOG_TYPE_DEBUG,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ESCAPE_INFO(szMsg, ...)      CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_INFO  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,   CYLOGGER_NAMESPACE::LOG_TYPE_INFO,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ESCAPE_WARN(szMsg, ...)      CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_WARN  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,   CYLOGGER_NAMESPACE::LOG_TYPE_WARN,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ESCAPE_ERROR(szMsg, ...)     CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_ERROR | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,          CYLOGGER_NAMESPACE::LOG_TYPE_ERROR,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ESCAPE_FATAL(szMsg, ...)     CY_LOG()->WriteEscapeLog(CYLOGGER_NAMESPACE::LOG_LEVEL_FATAL | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,          CYLOGGER_NAMESPACE::LOG_TYPE_FATAL,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)

#define CY_LOG_TRACE(szMsg, ...)            CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_DEBUG(szMsg, ...)            CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_DEBUG | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_DEBUG,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_INFO(szMsg, ...)             CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_INFO  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_INFO,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_WARN(szMsg, ...)             CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_WARN  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_WARN,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_ERROR(szMsg, ...)            CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_ERROR | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,                CYLOGGER_NAMESPACE::LOG_TYPE_ERROR,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)
#define CY_LOG_FATAL(szMsg, ...)            CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_FATAL | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,                CYLOGGER_NAMESPACE::LOG_TYPE_FATAL,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, ##__VA_ARGS__)

#define CY_LOG_HEX_TRACE(szMsg, nLen)       CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,      CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)
#define CY_LOG_HEX_DEBUG(szMsg, nLen)       CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_DEBUG | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,      CYLOGGER_NAMESPACE::LOG_TYPE_DEBUG,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)
#define CY_LOG_HEX_INFO(szMsg, nLen)        CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_INFO  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,      CYLOGGER_NAMESPACE::LOG_TYPE_INFO,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)
#define CY_LOG_HEX_WARN(szMsg, nLen)        CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_WARN  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,      CYLOGGER_NAMESPACE::LOG_TYPE_WARN,      UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)
#define CY_LOG_HEX_ERROR(szMsg, nLen)       CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_ERROR | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,             CYLOGGER_NAMESPACE::LOG_TYPE_ERROR,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)
#define CY_LOG_HEX_FATAL(szMsg, nLen)       CY_LOG()->WriteHexLog(CYLOGGER_NAMESPACE::LOG_LEVEL_FATAL | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,             CYLOGGER_NAMESPACE::LOG_TYPE_FATAL,     UNKNOWN_SEVER_CODE, __TFILE__, __TFUNCTION__, __TLINE__, szMsg, nLen)

#define CY_LOG_DIRECT_TRACE(szMsg)          CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, szMsg)
#define CY_LOG_DIRECT_DEBUG(szMsg)          CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_DEBUG | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_DEBUG,     UNKNOWN_SEVER_CODE, szMsg)
#define CY_LOG_DIRECT_INFO(szMsg)           CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_INFO  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_INFO,      UNKNOWN_SEVER_CODE, szMsg)
#define CY_LOG_DIRECT_WARN(szMsg)           CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_WARN  | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL,         CYLOGGER_NAMESPACE::LOG_TYPE_WARN,      UNKNOWN_SEVER_CODE, szMsg)
#define CY_LOG_DIRECT_ERROR(szMsg)          CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_ERROR | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,                CYLOGGER_NAMESPACE::LOG_TYPE_ERROR,     UNKNOWN_SEVER_CODE, szMsg)
#define CY_LOG_DIRECT_FATAL(szMsg)          CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_FATAL | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_TAIL,                CYLOGGER_NAMESPACE::LOG_TYPE_FATAL,     UNKNOWN_SEVER_CODE, szMsg)

// Trace.
//////////////////////////////////////////////////////////////////////////
#define CY_TRACE()                          CY_LOG_TRACE(TEXT("[TRACE]"))

// Scope.
//////////////////////////////////////////////////////////////////////////
class CYScope
{
public:
    template <typename... Args>
    CYScope(const TChar* pszFile /*= __TFILE__*/, const TChar* pszFun /*= __TFUNCTION__*/, int nLine /*= __TLINE__*/, const TChar* pszMsg, Args ...args) 
        :m_pszFile(pszFile), m_pszFun(pszFun), m_nLine(nLine)
    {
        CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL, CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, m_pszFile, m_pszFun, m_nLine, pszMsg, std::forward<Args>(args) ...);
    }

    ~CYScope()
    {
        CY_LOG()->WriteLog(CYLOGGER_NAMESPACE::LOG_LEVEL_TRACE | CYLOGGER_NAMESPACE::LOG_LEVEL_CONSOLE | LOG_LEVEL_REMOTE_TAIL, CYLOGGER_NAMESPACE::LOG_TYPE_TRACE,     UNKNOWN_SEVER_CODE, m_pszFile, m_pszFun, m_nLine, TEXT("[Scope] Leave."));
    }
private:
    int m_nLine = 0; const TChar* m_pszFile = nullptr; const TChar* m_pszFun = nullptr;
};

#define CY_SCOPE() CYScope objScope(__TFILE__, __TFUNCTION__, __TLINE__, TEXT("[Scope] Enter."));                          

#endif // __I_CYLOGGER_HPP__
