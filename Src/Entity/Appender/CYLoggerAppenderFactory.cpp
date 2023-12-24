#include "Src/Entity/Appender/CYLoggerAppenderFactory.hpp"
#include "Src/Entity/Appender/CYLoggerFileAppender.hpp"
#include "Src/Entity/Appender/CYLoggerMainAppender.hpp"
#include "Src/Entity/Appender/CYLoggerConsoleAppender.hpp"
#include "Src/Entity/Appender/CYLoggerRemoteAppender.hpp"
#include "Src/Entity/Appender/CYLoggerSystemAppender.hpp"
#include "Src/Config/CYLoggerConfig.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"

CYLOGGER_NAMESPACE_BEGIN

SharePtr<CYLoggerBaseAppender> CYLoggerAppenderFactory::CreateFileAppender(const TString& strFileName, ELogFileMode eFileMode, ELogType eLogType)
{
	SharePtr<CYLoggerBaseAppender> ptrAppender;
	switch (eLogType)
	{
	case LOG_TYPE_NONE:
		ptrAppender = MakeShared<CYLoggerConsoleAppender>(TEXT("CryLogger"), LoggerConfig()->GetShowConsoleWindow());
		break;
	case LOG_TYPE_TRACE:
		ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
		break;
    case LOG_TYPE_DEBUG:
        ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
        break;
	case LOG_TYPE_INFO:
		ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
		break;
	case LOG_TYPE_WARN:
		ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
		break;
	case LOG_TYPE_ERROR:
		ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
		break;
	case LOG_TYPE_FATAL:
		ptrAppender = MakeShared<CYLoggerFileAppender>(strFileName, eFileMode, eLogType);
		break;
    case LOG_TYPE_MAIN:
        ptrAppender = MakeShared<CYLoggerMainAppender>(strFileName, eFileMode, eLogType);
        break;
	case LOG_TYPE_REMOTE:
		ptrAppender = MakeShared<CYLoggerRemoteAppender>(strFileName);
		break;
	case LOG_TYPE_SYS:
		ptrAppender = MakeShared<CYLoggerSystemAppender>(strFileName);
		break;
	default:
		IfTrueThrow(true, TEXT("eLogType wrong type!"));
		break;
	}

	return ptrAppender;
}

CYLOGGER_NAMESPACE_END