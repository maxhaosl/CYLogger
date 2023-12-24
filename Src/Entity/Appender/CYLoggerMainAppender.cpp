#include "CYCoroutine/CYCoroutine.hpp"
#include "Src/Entity/Appender/CYLoggerMainAppender.hpp"
#include "Src/Statistics/CYStatistics.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerMainAppender::CYLoggerMainAppender(const TString& strFileName, ELogFileMode eFileMode, ELogType eLogType)
	: CYLoggerBufferAppender("MainThread")
	, CYFileRestriction()
	, m_bForceNewFile(false)
{
	this->m_eFileMode = eFileMode;
	IfTrueThrow(strFileName.length() == 0, TEXT("fileName cannot be empty"));

	TChar* p = cy_fullpath(nullptr, strFileName.c_str(), 0);
	IfTrueThrow(p == nullptr, TEXT("_fullpath() failed"));
	this->m_strFileName = p;
	delete p;

	this->m_eLogType = eLogType;

	StartLogThread();
}

CYLoggerMainAppender::~CYLoggerMainAppender()
{
	StopLogThread();
}

/**
* @brief Open log file.
*/
void CYLoggerMainAppender::OpenFile(int flags)
{
#if CY_USE_UNICODE
	//m_objLogFile.imbue(std::locale("chs"));
	m_objLogFile.imbue(std::locale(std::locale::classic(), ".OCP", std::locale::ctype | std::locale::collate));
#endif
	m_objLogFile.open(m_strFileName.c_str(), flags);

	IfTrueThrow(!m_objLogFile, TString(TEXT("cannot open file ")) += m_strFileName);
	if ((flags & TIos::app) == TIos::app)
	{
		m_objLogFile.seekp(0, std::ios_base::end);
		IfTrueThrow(!m_objLogFile, TString(TEXT("seekp() failed ")) += m_strFileName);
	}
}

/**
 * @brief int nFlags.
*/
void CYLoggerMainAppender::ReOpenFile(const TString& strNewFile)
{
    Flush();
    this->m_strFileName = strNewFile;
    OnActivate();
}

/**
 * @brief Create New Log File.
*/
void CYLoggerMainAppender::CreateNewLogFile()
{
    if (!m_bForceNewFile)
        return;

    m_bForceNewFile = false;

    TString strData = CYTimeStamps().GetTimeStr();
    TString strFileExt = CYPublicFunction::GetFileExt(this->m_strFileName);
    TString strFileBase = CYPublicFunction::GetBasePath(this->m_strFileName);
    TString strNewLogFile = std::format(TEXT("{}_{}.{}"), strFileBase, strData, strFileExt);

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

    {
        LockGuard locker(m_mutex);
        m_objNewFilePromise.set_value(true);
    }
}

/**
* @brief Implementation of a pure virtual method from CYLoggerBaseAppender
*/
void CYLoggerMainAppender::Log(const TStringView& strMsg, int nTypeIndex, bool bFlush)
{
	CYFPSCounter::UpdateCounter();
	m_objLogFile << strMsg << TEXT("\n");
	IfTrueThrow(m_objLogFile.fail(), TString(TEXT("operator<<() failed for file ")) += m_strFileName);

	if (bFlush)
		Flush();

    Statistics()->AddMainLine(1);
    Statistics()->AddMainBytes(strMsg.size() + TEXT_BYTE_LEN);

    Statistics()->AddMainCurrentFPS(CYFPSCounter::GetCurrentFPS());
    Statistics()->AddMainAverageFPS(CYFPSCounter::GetAverageFPS());

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
void CYLoggerMainAppender::ForceNewFile()
{
    LockGuard locker(m_mutex);
    std::promise<bool> objNewFilePromise;
    m_objNewFilePromise.swap(objNewFilePromise);
    m_objNewFileFuture = m_objNewFilePromise.get_future();

	m_bForceNewFile = true;

    m_objNewFileFuture.get();
}

CYLOGGER_NAMESPACE_END