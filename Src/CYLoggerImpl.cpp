#include "Src/CYLoggerImpl.hpp"
#include "Src/CYLoggerControl.hpp"
#include "Src/Config/CYLoggerConfig.hpp"
#include "Src/Common/CYPublicFunction.hpp"
#include "Src/Common/Message/CYEscapeMessage.hpp"
#include "Src/Common/Message/CYNormalMessage.hpp"
#include "Src/Common/Message/CYStrMessage.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "Src/Common/Exception/CYExceptionLogFile.hpp"
#include "Src/Statistics/CYStatistics.hpp"

#include <cstdarg>
#include <strstream>
#include <iostream>
#include <iomanip>
#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////

namespace
{
	SharePtr<ICYLogger> g_ptrLogger;
}

ICYLogger* ICYLogger::GetInstance(ELogLevelFilter eLevelFilter)
{
	if (!g_ptrLogger)
	{
        EXCEPTION_BEGIN
        {
            g_ptrLogger = MakeShared<CYLLoggerImpl>(eLevelFilter);
            g_ptrLogger->Init();
        }
        EXCEPTION_END
	}

	return g_ptrLogger.get();
}

void ICYLogger::FreeInstance()
{
	EXCEPTION_BEGIN
	{
        if (g_ptrLogger)
        {
            g_ptrLogger->UnInit();
        }
		g_ptrLogger.reset();
        Statistics()->Reset();
	}
	EXCEPTION_END
}

//////////////////////////////////////////////////////////////////////////
CYLLoggerImpl::CYLLoggerImpl(ELogLevelFilter eLogFilterLevel/* = ELogLevelFilter::LOG_FILTER_ALL*/)
{
	EXCEPTION_BEGIN
	{
		m_ptrLoggerControl = MakeShared<CYLoggerControl>(eLogFilterLevel);
	}
	EXCEPTION_END
}

CYLLoggerImpl::~CYLLoggerImpl()
{
	EXCEPTION_BEGIN
	{
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_ptrLoggerControl)
            m_ptrLoggerControl->Flush(ELogType::LOG_TYPE_MAX);
        m_ptrLoggerControl.reset();
	}
	EXCEPTION_END
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief Initialization and de-initialization of cry Log.
*/
bool CYLLoggerImpl::Init() noexcept
{
	bool bRet = false;
	EXCEPTION_BEGIN
	{
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (LoggerConfig()->GetLogPath().empty())
            LoggerConfig()->SetLogPath(LoggerConfig()->GetWorkPath());

        ExceptionLogFile()->InitLog(EXCEPT_LOG_TIME, EXCEPT_LOG_LINE_COUNT, LoggerConfig()->GetLogPath().c_str(), LOG_DIR, LoggerConfig()->GetErrorLogName().c_str());

        bRet = m_ptrLoggerControl->Init();
	}
    EXCEPTION_END
    return bRet;
}

void CYLLoggerImpl::UnInit() noexcept
{
	EXCEPTION_BEGIN
	{
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        m_bExit = true;
        m_ptrLoggerControl->UnInit();

        ExceptionLogFile_Free();
	}
	EXCEPTION_END
}

/**
 * @brief Flush Log.
*/
void CYLLoggerImpl::Flush(ELogType eLogType) noexcept
{
	EXCEPTION_BEGIN
	{
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return;

        m_ptrLoggerControl->Flush(eLogType);
	}
	EXCEPTION_END
}

/**
 * @brief Append Log file.
*/
bool CYLLoggerImpl::AddApender(ELogType eLogType, const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode) noexcept
{
	bool bRet = false;
	EXCEPTION_BEGIN
	{
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));
        IfTrueThrow(szFile == nullptr && eLogType != LOG_TYPE_REMOTE && eLogType != LOG_TYPE_SYS, TEXT("szFile cannot be empty"));

        if (m_bExit)
            return false;

        bRet = m_ptrLoggerControl->AddApender(eLogType, szChannel, szFile, eFileMode);
	}
	EXCEPTION_END
    return bRet;
}

/**
 * @brief Write Log.
*/
void CYLLoggerImpl::WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) noexcept
{
	if (szMsg == nullptr)
		return;

	if (cy_strlen(szMsg) == 0)
		return;

	if (m_bExit)
		return;

    EXCEPTION_BEGIN
    {
        va_list args;
    #if defined(_WIN32) && CY_USE_UNICODE
        va_start(args, szMsg);
        int iLen = cy_vscprintf(szMsg, args) + 1;
        UniquePtr<TChar[]> ptrLogBuffer = MakeUnique<TChar[]>(iLen);
        cy_vsnprintf_s(ptrLogBuffer.get(), iLen, iLen, szMsg, args);
        va_end(args);
    #else
        va_start(args, szMsg);
        int iLen = vsnprintf(nullptr, 0, szMsg, args) + 1;
        va_end(args);
        UniquePtr<TChar[]> ptrLogBuffer = MakeUnique<TChar[]>(iLen);
        va_start(args, szMsg);
        vsnprintf(ptrLogBuffer.get(), iLen, szMsg, args);
        va_end(args);
    #endif

        TString strMsg = ptrLogBuffer.get();
        m_ptrLoggerControl->WriteLog(nLogLevel, MakeShared<CYNormalMessage>(TEXT(""), eMsgType, nSeverCode, strMsg, pszFile, pszFuncName, nLine));
    }
	EXCEPTION_END
}

void CYLLoggerImpl::WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* szMsg) noexcept
{
	if (szMsg == nullptr)
		return;

	if (cy_strlen(szMsg) == 0)
		return;

	if (m_bExit)
		return;

	EXCEPTION_BEGIN
	{
        TString strMsg = szMsg;
        m_ptrLoggerControl->WriteLog(nLogLevel, MakeShared<CYStrMessage>(TEXT(""), eMsgType, nSeverCode, strMsg));
	}
	EXCEPTION_END
}

void CYLLoggerImpl::WriteEscapeLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) noexcept
{
	if (szMsg == nullptr)
		return;

	if (cy_strlen(szMsg) == 0)
		return;

	if (m_bExit)
		return;

	EXCEPTION_BEGIN
	{
        va_list args;
    #if defined(_WIN32) && CY_USE_UNICODE
        va_start(args, szMsg);
        int iLen = cy_vscprintf(szMsg, args) + 1;
        UniquePtr<TChar[]> ptrLogBuffer = MakeUnique<TChar[]>(iLen);
        cy_vsnprintf_s(ptrLogBuffer.get(), iLen, iLen, szMsg, args);
        va_end(args);
    #else
        va_start(args, szMsg);
        int iLen = vsnprintf(nullptr, 0, szMsg, args) + 1;
        va_end(args);
        UniquePtr<TChar[]> ptrLogBuffer = MakeUnique<TChar[]>(iLen);
        va_start(args, szMsg);
        vsnprintf(ptrLogBuffer.get(), iLen, szMsg, args);
        va_end(args);
    #endif

        TString strMsg = ptrLogBuffer.get();
        m_ptrLoggerControl->WriteLog(nLogLevel, MakeShared<CYEscapeMessage>(TEXT(""), eMsgType, nSeverCode, strMsg, pszFile, pszFuncName, nLine));
	}
	EXCEPTION_END
}

void CYLLoggerImpl::WriteHexLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const void* szMsg, int nLen) noexcept
{
	if (szMsg == nullptr)
		return;

	if (m_bExit)
		return;

	EXCEPTION_BEGIN
	{
        char* pszMsg = (char*)szMsg;

        TOStringStream msgStream;
        msgStream << TEXT("address[0x") << std::hex << std::uppercase << std::setw(8) << std::setfill(TEXT('0')) << (void*)pszMsg << TEXT("] size[") << std::dec << nLen << TEXT("]") << std::endl;

        char cLine[16];
        int nHexLine = 1;
        int nLineSize = 0;
        for (int nPos = 0; nPos < nLen; nHexLine++)
        {
            nLineSize = min(nLen - nPos, 16);
            memcpy(cLine, pszMsg + nPos, nLineSize);
            nPos += nLineSize;
            msgStream << TEXT("[") << std::dec << std::uppercase << std::setw(2) << std::setfill(TEXT('0')) << nHexLine << TEXT("]");
            for (int n = 0; n < nLineSize; n++)
            {
                if (n == 8) msgStream << TEXT(" ");
                msgStream << std::dec << std::uppercase << std::setw(2) << std::setfill(TEXT('0')) << (cLine[n] & 0x00FF);
            }

            for (int n = nLineSize; n < 16; n++)
            {
                if (n == 8) msgStream << TEXT(" ");
                msgStream << TEXT("  ");
            }
            msgStream << TEXT(" :");

            for (int n = 0; n < nLineSize; n++)
            {
                if (!isprint(cLine[n])) cLine[n] = TEXT('.');
                msgStream << cLine[n];
            }
            msgStream << std::endl;
        }

        TString strMsg = msgStream.str();
        m_ptrLoggerControl->WriteLog(nLogLevel, MakeShared<CYNormalMessage>(TEXT(""), eMsgType, nSeverCode, strMsg, pszFile, pszFuncName, nLine));
	}
    EXCEPTION_END
}

/**
 * @brief Set Log Config.
*/
void CYLLoggerImpl::SetConfig(const TChar* szLogPath, bool bShowConsoleWindow) noexcept
{
    EXCEPTION_BEGIN
    {
        if (szLogPath == nullptr || cy_strlen(szLogPath) == 0)
            LoggerConfig()->SetLogPath(LoggerConfig()->GetWorkPath());
        else
            LoggerConfig()->SetLogPath(szLogPath);

        LoggerConfig()->SetShowConsoleWindow(bShowConsoleWindow);
    }
    EXCEPTION_END
}

/**
 * @brief Set restriction rules
*/
void CYLLoggerImpl::SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize) noexcept
{
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return;

        m_ptrLoggerControl->SetRestriction(bEnableCheck, m_bClearUnLogFile, nLimitTimeClearLog, nLimitTimeExpiredFile, nCheckFileSizeTime, nCheckFileCountTime, nCheckFileSize, nFileCountPerType, nCheckFileTypeSize, nCheckALLFileSize);
    }
    EXCEPTION_END
}

/**
 * @brief Log special character filtering.
 * @param pFilter needs to be saved globally, and its scope is larger than the log library instance.
*/
void CYLLoggerImpl::SetFilter(ICYLoggerPatternFilter* pFilter)  noexcept
{
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return;

        m_ptrLoggerControl->SetFilter(pFilter);
    }
    EXCEPTION_END
}

/**
 * @brief Log information format template.
 * @param pLayout needs to be saved globally, and its scope is larger than the log library instance.
*/
void CYLLoggerImpl::SetLayout(ELogLayoutType eLayoutType, ICYLoggerTemplateLayout* pLayout)  noexcept
{
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return;

        LoggerConfig()->SetLayoutType(eLayoutType);
        m_ptrLoggerControl->SetLayout(pLayout);
    }
        EXCEPTION_END
}

/**
    * @brief Get statistics.
*/
bool CYLLoggerImpl::GetStats(STStatistics* pStats) noexcept
{
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return false;

        return m_ptrLoggerControl->GetStats(pStats);
    }
    EXCEPTION_END
    return false;
}

/**
 * @brief Get Log Filter Level.
*/
ELogLevelFilter CYLLoggerImpl::GetLogFilterLevel() const noexcept
{
    ELogLevelFilter eRet = LOG_FILTER_NONE;
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));
        eRet = m_ptrLoggerControl->GetLogFilterLevel();
    }
    EXCEPTION_END
    return eRet;
}

/**
 * @brief Set Log Filter Level.
*/
void CYLLoggerImpl::SetLogFilterLevel(ELogLevelFilter eLogFilterLevel) noexcept
{
    EXCEPTION_BEGIN
    {
        IfTrueThrow(!m_ptrLoggerControl, TEXT("m_ptrLoggerControl cannot be empty"));

        if (m_bExit)
            return;

        m_ptrLoggerControl->SetLogFilterLevel(eLogFilterLevel);
    }
    EXCEPTION_END
}

CYLOGGER_NAMESPACE_END