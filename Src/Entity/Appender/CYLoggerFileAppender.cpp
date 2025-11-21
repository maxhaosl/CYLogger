#include "Entity/Appender/CYLoggerFileAppender.hpp"
#include "Statistics/CYStatistics.hpp"
#include "Common/CYFormatDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerFileAppender::CYLoggerFileAppender(const TString& strFileName, ELogFileMode eFileMode, ELogType eLogType)
    : CYLoggerBaseAppender("CYLoggerFileAppender")
    , CYFileRestriction()
    , m_bForceNewFile(false)
{
    this->m_eFileMode = eFileMode;
    IfTrueThrow(strFileName.length() == 0, TEXT("fileName cannot be empty"));

    UniquePtr<TChar> ptrFileName;
    TChar* p = cy_fullpath(nullptr, strFileName.c_str(), 0);
    ptrFileName.reset(p);
    IfTrueThrow(p == nullptr, TEXT("_fullpath() failed"));
    this->m_strFileName = p;

    //std::transform(this->fileName.begin(), this->fileName.end(), this->fileName.begin(), tolower);
    this->m_eLogType = eLogType;

    switch (m_eLogType)
    {
    case LOG_TYPE_NONE:
        SetThreadName("ConsoleThread");
        break;
    case LOG_TYPE_TRACE:
        SetThreadName("TraceThread");
        break;
    case LOG_TYPE_DEBUG:
        SetThreadName("DebugThread");
        break;
    case LOG_TYPE_INFO:
        SetThreadName("InfoThread");
        break;
    case LOG_TYPE_WARN:
        SetThreadName("WarnThread");
        break;
    case LOG_TYPE_ERROR:
        SetThreadName("ErrorThread");
        break;
    case LOG_TYPE_FATAL:
        SetThreadName("FatalThread");
        break;
    case LOG_TYPE_MAIN:
        SetThreadName("MainThread");
        break;
    case LOG_TYPE_REMOTE:
        SetThreadName("RemoteThread");
        break;
    case LOG_TYPE_SYS:
        SetThreadName("SysThread");
        break;
    case LOG_TYPE_MAX:
        assert(0);
    default:
        assert(0);
        break;
    }

    StartLogThread();
}

CYLoggerFileAppender::~CYLoggerFileAppender()
{
    StopLogThread();
}

void CYLoggerFileAppender::OpenFile(int nFlags)
{
#if CY_USE_UNICODE
    //m_objLogFile.imbue(std::locale("chs"));
    m_objLogFile.imbue(std::locale(std::locale::classic(), ".OCP", std::locale::ctype | std::locale::collate));
#endif
    m_objLogFile.open(m_strFileName.c_str(), nFlags);

    IfTrueThrow(!m_objLogFile, TString(TEXT("cannot open file ")) += m_strFileName);
    if ((nFlags & TIos::app) == TIos::app)
    {
        m_objLogFile.seekp(0, std::ios_base::end);
        IfTrueThrow(!m_objLogFile, TString(TEXT("seekp() failed ")) += m_strFileName);
    }
}

/**
 * @brief int nFlags.
*/
void CYLoggerFileAppender::ReOpenFile(const TString& strNewFile)
{
    Flush();
    this->m_strFileName = strNewFile;
    OnActivate();
}

/**
 * @brief Create New Log File.
*/
void CYLoggerFileAppender::CreateNewLogFile()
{
    if (!m_bForceNewFile)
        return;

    m_bForceNewFile = false;

    TString strData = CYTimeStamps().GetTimeStr();
    TString strFileExt = CYPublicFunction::GetFileExt(this->m_strFileName);
    TString strFileBase = CYPublicFunction::GetBasePath(this->m_strFileName);
    TString strNewLogFile = strFileBase + TEXT("_") + strData + TEXT(".") + strFileExt;

    switch (m_eFileMode)
    {
    case LOG_MODE_FILE_APPEND:
        Copy(strNewLogFile);
        break;
    case LOG_MODE_FILE_TIME:
        ReOpenFile(strNewLogFile);
        break;
    default:
        assert(0);
        break;
    }

    CYFileRestriction::ResetFileSize();

    {
        LockGuard locker(m_mutex);
        m_objNewFilePromise.set_value(true);
    }
}

/**
* @brief Implementation of a pure virtual method from CYLoggerBaseAppender
*/
void CYLoggerFileAppender::Log(const TStringView& strMsg, int nTypeIndex, bool bFlush)
{
    CYFPSCounter::UpdateCounter();
    m_objLogFile << strMsg << TEXT("\n");
    IfTrueThrow(m_objLogFile.fail(), TString(TEXT("operator<<() failed for file ")) += m_strFileName);

    if (bFlush)
        Flush();

    switch (m_eLogType)
    {
    case LOG_TYPE_NONE:
        Statistics()->AddConsoleLine(1);
        Statistics()->AddConsoleBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddConsoleCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddConsoleAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_TRACE:
        Statistics()->AddTraceLine(1);
        Statistics()->AddTraceBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddTraceCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddTraceAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_DEBUG:
        Statistics()->AddDebugLine(1);
        Statistics()->AddDebugBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddDebugCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddDebugAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_INFO:
        Statistics()->AddInfoLine(1);
        Statistics()->AddInfoBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddInfoCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddInfoAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_WARN:
        Statistics()->AddWarnLine(1);
        Statistics()->AddWarnBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddWarnCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddWarnAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_ERROR:
        Statistics()->AddErrorLine(1);
        Statistics()->AddErrorBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddErrorCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddErrorAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_FATAL:
        Statistics()->AddFatalLine(1);
        Statistics()->AddFatalBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddFatalCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddFatalAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_MAIN:
        Statistics()->AddMainLine(1);
        Statistics()->AddMainBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddMainCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddMainAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_REMOTE:
        Statistics()->AddRemoteLine(1);
        Statistics()->AddRemoteBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddRemoteCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddRemoteAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_SYS:
        Statistics()->AddSysLine(1);
        Statistics()->AddSysBytes(strMsg.size() + TEXT_BYTE_LEN);
        Statistics()->AddSysCurrentFPS(CYFPSCounter::GetCurrentFPS());
        Statistics()->AddSysAverageFPS(CYFPSCounter::GetAverageFPS());
        break;
    case LOG_TYPE_MAX:
        assert(0);
    default:
        assert(0);
        break;
    }

    if (this->m_eFileMode == ELogFileMode::LOG_MODE_FILE_TIME)
    {
        CYFileRestriction::AddFileSize(strMsg.length() + 1);
        if (CYFileRestriction::IsCreateNewLog())
        {
            ReOpenFile(CYFileRestriction::GetNewLogName(this->m_strFileName));
            CYFileRestriction::ResetFileSize();
        }
    }

    CreateNewLogFile();
}

/**
 * @brief Force new log file.
*/
void CYLoggerFileAppender::ForceNewFile()
{
    LockGuard locker(m_mutex);
    std::promise<bool> objNewFilePromise;
    m_objNewFilePromise.swap(objNewFilePromise);
    m_objNewFileFuture = m_objNewFilePromise.get_future();

    m_bForceNewFile = true;

    m_objNewFileFuture.get();
}

CYLOGGER_NAMESPACE_END
