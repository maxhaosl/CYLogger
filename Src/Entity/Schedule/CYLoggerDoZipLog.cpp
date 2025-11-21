#include "Entity/Schedule/CYLoggerDoZipLog.hpp"
#include "Entity/CYLoggerEntityFactory.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerDoZipLog::CYLoggerDoZipLog()
{

}

CYLoggerDoZipLog::~CYLoggerDoZipLog()
{

}

void CYLoggerDoZipLog::SetConfig()
{

}

void CYLoggerDoZipLog::Process()
{
    // Before compressing the log file, force the creation of a new log file. The log file currently in use may have read permission issues.
    LoggerEntityFactory()->ForceEntityNewFile();




}

CYLOGGER_NAMESPACE_END