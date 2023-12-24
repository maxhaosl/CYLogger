#include "Src/Entity/Appender/CYLoggerSystemAppender.hpp"
#include "Src/Entity/Appender/CYLoggerAppenderDefine.hpp"
#include "Src/Statistics/CYStatistics.hpp"

#if defined(CYLOGGER_LINUX_OS) || defined(CYLOGGER_MAC_OS)
#include <syslog.h>
#endif

CYLOGGER_NAMESPACE_BEGIN

namespace
{
	void WriteToEventLog(const TString& strName, LPCTSTR message, WORD eventType)
	{
		HANDLE hEventLog = RegisterEventSource(NULL, strName.c_str());

		if (hEventLog != NULL)
		{
			LPCTSTR strings[2] = { message, NULL };

			ReportEvent(hEventLog, eventType, 0, 0, NULL, 1, 0, strings, NULL);

			DeregisterEventSource(hEventLog);
		}
	}
}

CYLoggerSystemAppender::CYLoggerSystemAppender(const TString& strName) noexcept
: CYLoggerBaseAppender("SysThread")
, m_strAppName(strName)
{
    StartLogThread();
}

CYLoggerSystemAppender::~CYLoggerSystemAppender() noexcept
{
    StopLogThread();
}

/**
* @return the unique id for this appender
*/
const ELogType CYLoggerSystemAppender::GetId() const
{
    return ELogType::LOG_TYPE_SYS;
}

/**
* Outputs message on the physical device
* @param msg message to output
* @bFlush if true flushing may be done right after the output. It's for concrete implementation to respect this parameter
*/
void CYLoggerSystemAppender::Log(const TStringView& strMsg, int nTypeIndex, bool bFlush)
{
	CYFPSCounter::UpdateCounter();
	LPCTSTR logMessage = strMsg.data();

#ifdef WIN32
	WORD eventType = EVENTLOG_INFORMATION_TYPE;
	switch (strMsg[nTypeIndex])
	{
	case 'I': eventType = EVENTLOG_INFORMATION_TYPE;	break;
	case 'D': eventType = EVENTLOG_INFORMATION_TYPE;	break;
	case 'T': eventType = EVENTLOG_INFORMATION_TYPE;	break;
	case 'W': eventType = EVENTLOG_WARNING_TYPE;		break;
	case 'E': eventType = EVENTLOG_ERROR_TYPE;			break;
	case 'F': eventType = EVENTLOG_ERROR_TYPE;			break;
	default:  eventType = EVENTLOG_INFORMATION_TYPE;	break;
	}

	WriteToEventLog(m_strAppName, logMessage, eventType);
#elif defined(CYLOGGER_LINUX_OS) || defined(CYLOGGER_MAC_OS)
    openlog("CYLogger", LOG_PID | LOG_CONS, LOG_USER);

	int eventType = LOG_INFO;
    switch (strMsg[nTypeIndex])
    {
    case 'I': eventType = LOG_INFO;			break;
    case 'D': eventType = LOG_DEBUG;		break;
    case 'T': eventType = LOG_DEBUG;		break;
    case 'W': eventType = LOG_WARNING;		break;
    case 'E': eventType = LOG_ERR;			break;
    case 'F': eventType = LOG_ERR;			break;
    default:  eventType = LOG_INFO;			break;
    }

    syslog(eventType, logMessage);
    closelog();
#endif

    Statistics()->AddSysLine(1);
    Statistics()->AddSysBytes(strMsg.size() + TEXT_BYTE_LEN);

    Statistics()->AddSysCurrentFPS(CYFPSCounter::GetCurrentFPS());
    Statistics()->AddSysAverageFPS(CYFPSCounter::GetAverageFPS());
}

/**
* Get called when this appender become active through attaching to a CYLLoggerImpl object, and it was not active before
*/
void CYLoggerSystemAppender::OnActivate()
{
	return;
}

/**
* Attempt to Flush buffers to a physical device - default implementation does nothing
*/
void CYLoggerSystemAppender::Flush()
{
	return;
}

/**
* Returns the actual size of data on a physical device - default implementation does not respect this and throws an exception
* @returns the actual size of data, located on a physical device
*/
//virtual long GetSize() {
int64_t CYLoggerSystemAppender::GetSize()
{
    return 0;
}

/**
* Attempt to physically copy data - default implementation does not respect this and throws an exception
*/
void CYLoggerSystemAppender::Copy(const TString& strTarget)
{
	return;
}

/**
* Attempt to physically clear data - default implementation does not respect this and throws an exception
*/
void CYLoggerSystemAppender::ClearContents()
{
	return;
}

/**
 * @brief Get log file name.
*/
const TString& CYLoggerSystemAppender::GetLogName()
{
	return m_strLogName;
}

/**
 * @brief Force new log file.
*/
void CYLoggerSystemAppender::ForceNewFile()
{

}

CYLOGGER_NAMESPACE_END