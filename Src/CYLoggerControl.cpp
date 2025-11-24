#include "CYLoggerControl.hpp"
#include "Entity/CYLoggerEntityFactory.hpp"
#include "Common/CYPathConvert.hpp"
#include "Config/CYLoggerConfig.hpp"
#include "Statistics/CYStatistics.hpp"
#include "Entity/Schedule/CYLoggerSchedule.hpp"
#include "Common/CYFileRestriction.hpp"
#include "Entity/Filter/CYLoggerPatternFilterManager.hpp"
#include "Entity/Layout/CYLoggerTemplateLayoutManager.hpp"
#include <cstdlib>

CYLOGGER_NAMESPACE_BEGIN

CYLoggerControl::CYLoggerControl(ELogLevelFilter eLogFilterLevel/* = ELogLevelFilter::LOG_FILTER_ALL*/)
: m_eLogFilterLevel(eLogFilterLevel)
{

}

CYLoggerControl::~CYLoggerControl()
{

}

/**
 * @brief Initialization and de-initialization of cry Log.
*/
bool CYLoggerControl::Init()
{
	m_ptrSchedule = MakeUnique<CYLoggerSchedule>("ScheduleThread");
	IfTrueThrow(nullptr == m_ptrSchedule, TEXT("m_ptrSchedule cannot be empty."));
	return true;
}

void CYLoggerControl::UnInit()
{
	if (m_ptrSchedule)
	{
		m_ptrSchedule->StopSchedule();
		m_ptrSchedule.reset();
	}
	for (auto& ptrEntity : m_arrayEntity)
	{
		ptrEntity.reset();
	}

	LoggerEntityFactory()->ReleaseAllLoggerEntity();
}

/**
 * @brief Flush Log.
*/
void CYLoggerControl::Flush(ELogType eLogType)
{
	if (eLogType == ELogType::LOG_TYPE_MAX)
	{
		for (auto& ptrEntity : m_arrayEntity)
		{
			if(ptrEntity) ptrEntity->Flush();
		}
		return;
	}
	
	IfTrueThrow(nullptr == m_arrayEntity[eLogType], TEXT("m_arrayEntity[eLogType] was empty."));
	m_arrayEntity[eLogType]->Flush();
}

/**
 * @brief Append Log file.
*/
bool CYLoggerControl::AddApender(ELogType eLogType, const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode)
{
	IfTrueThrow(nullptr == m_ptrSchedule, TEXT("m_ptrSchedule cannot be empty."));
	m_ptrSchedule->AddLogType(eLogType);

	TString strLogFile = CYPathConvert::GetLogFilePath(szChannel, szFile, eFileMode);
	TString strLogPath = CYPathConvert::ConvertFilePath(strLogFile.c_str(), LoggerConfig()->GetLogPath().c_str(), LOG_DIR);
	m_arrayEntity[eLogType] = CYLoggerEntityFactory::CreateEntity(eLogType, strLogPath, eFileMode);

	IfTrueThrow(nullptr == m_arrayEntity[eLogType], TEXT("CreateEntity failed!"));
	return nullptr != m_arrayEntity[eLogType];
}

/**
* @brief Put message to the Log
*/
void CYLoggerControl::WriteLog(int nLogLevel, const SharePtr<CYBaseMessage>& ptrMessage)
{
    IfTrueThrow(!ptrMessage, TEXT("ptrMessage cannot be empty"));

	if (0 == (m_eLogFilterLevel & nLogLevel))
		return;

	if (!m_arrayEntity[LOG_TYPE_NONE])
	{
		m_arrayEntity[LOG_TYPE_NONE] = CYLoggerEntityFactory::CreateEntity(ELogType::LOG_TYPE_NONE, TString(), ELogFileMode::LOG_MODE_FILE_APPEND);
	}

	if ((nLogLevel & LOG_LEVEL_CONSOLE) > 0 && m_arrayEntity[LOG_TYPE_NONE])
	{
		m_arrayEntity[LOG_TYPE_NONE]->WriteLog(ptrMessage);
	}

    if ((nLogLevel & LOG_LEVEL_TRACE) > 0 && m_arrayEntity[LOG_TYPE_TRACE])
    {
        m_arrayEntity[LOG_TYPE_TRACE]->WriteLog(ptrMessage);
    }

	if ((nLogLevel & LOG_LEVEL_DEBUG) > 0 && m_arrayEntity[LOG_TYPE_DEBUG])
	{
		m_arrayEntity[LOG_TYPE_DEBUG]->WriteLog(ptrMessage);
	}

	if ((nLogLevel & LOG_LEVEL_INFO) > 0 && m_arrayEntity[LOG_TYPE_INFO])
	{
		m_arrayEntity[LOG_TYPE_INFO]->WriteLog(ptrMessage);
	}

	if ((nLogLevel & LOG_LEVEL_WARN) > 0 && m_arrayEntity[LOG_TYPE_WARN])
	{
		m_arrayEntity[LOG_TYPE_WARN]->WriteLog(ptrMessage);
	}

	if ((nLogLevel & LOG_LEVEL_ERROR) > 0 && m_arrayEntity[LOG_TYPE_ERROR])
	{
		m_arrayEntity[LOG_TYPE_ERROR]->WriteLog(ptrMessage);
	}

	if ((nLogLevel & LOG_LEVEL_FATAL) > 0 && m_arrayEntity[LOG_TYPE_FATAL])
	{
		m_arrayEntity[LOG_TYPE_FATAL]->WriteLog(ptrMessage);
	}

    if (m_arrayEntity[LOG_TYPE_MAIN])
    {
        m_arrayEntity[LOG_TYPE_MAIN]->WriteLog(ptrMessage);
    }

	if ((nLogLevel & LOG_LEVEL_REMOTE) > 0 && m_arrayEntity[LOG_TYPE_REMOTE])
	{
		m_arrayEntity[LOG_TYPE_REMOTE]->WriteLog(ptrMessage);
	}

	if ((nLogLevel & LOG_LEVEL_SYS) > 0 && m_arrayEntity[LOG_TYPE_SYS])
	{
		m_arrayEntity[LOG_TYPE_SYS]->WriteLog(ptrMessage);
	}
}

/**
 * @brief Get Log Level.
*/
ELogLevelFilter CYLoggerControl::GetLogFilterLevel() const
{
	return m_eLogFilterLevel;
}

void CYLoggerControl::SetLogFilterLevel(ELogLevelFilter eLogFilterLevel)
{
	m_eLogFilterLevel = eLogFilterLevel;
}

/**
 * @brief Set restriction rules
*/
void CYLoggerControl::SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize)
{
	IfTrueThrow(!m_ptrSchedule, TEXT("m_ptrSchedule cannot be empty"));

    const char* pszEnvDisable = std::getenv("CYLOGGER_DISABLE_RESTRICTIONS");
    if (pszEnvDisable && pszEnvDisable[0] && pszEnvDisable[0] != '0')
    {
        return;
    }

	CYFileRestriction::SetRestriction(nCheckFileSize);
    m_ptrSchedule->SetRestriction(bEnableCheck, m_bClearUnLogFile, nLimitTimeClearLog, nLimitTimeExpiredFile, nCheckFileSizeTime, nCheckFileCountTime, nCheckFileSize, nFileCountPerType, nCheckFileTypeSize, nCheckALLFileSize);

    if (bEnableCheck) m_ptrSchedule->StartSchedule();
}

/**
 * @brief Log special character filtering.
 * @param pFilter needs to be saved globally, and its scope is larger than the log library instance.
*/
void CYLoggerControl::SetFilter(ICYLoggerPatternFilter* pFilter)
{
    LoggerPatternFilterManager()->SetNextFilter(pFilter);
}

/**
 * @brief Log information format template.
 * @param pLayout needs to be saved globally, and its scope is larger than the log library instance.
*/
void CYLoggerControl::SetLayout(ICYLoggerTemplateLayout* pLayout)
{
    LoggerTemplateLayoutManager()->SetCustomLayout(pLayout);
}

/**
  * @brief Get statistics.
*/
bool CYLoggerControl::GetStats(STStatistics* pStats)
{
    IfTrueThrow(!pStats, TEXT("pStats cannot be empty"));

	pStats->nTotalByte = Statistics()->GetTotalBytes();
	pStats->nTotalLine = Statistics()->GetTotalLine();

    pStats->nConsoleByte = Statistics()->GetConsoleBytes();
    pStats->nConsoleLine = Statistics()->GetConsoleLine();
    pStats->fConsoleCurrentFps = Statistics()->GetConsoleCurrentFPS();
    pStats->fConsoleAverageFps = Statistics()->GetConsoleAverageFPS();

    pStats->nTraceByte = Statistics()->GetTraceBytes();
    pStats->nTraceLine = Statistics()->GetTraceLine();
    pStats->fTraceCurrentFps = Statistics()->GetTraceCurrentFPS();
    pStats->fTraceAverageFps = Statistics()->GetTraceAverageFPS();

    pStats->nDebugByte = Statistics()->GetDebugBytes();
    pStats->nDebugLine = Statistics()->GetDebugLine();
    pStats->fDebugCurrentFps = Statistics()->GetDebugCurrentFPS();
    pStats->fDebugAverageFps = Statistics()->GetDebugAverageFPS();

    pStats->nInfoByte = Statistics()->GetInfoBytes();
    pStats->nInfoLine = Statistics()->GetInfoLine();
    pStats->fInfoCurrentFps = Statistics()->GetInfoCurrentFPS();
    pStats->fInfoAverageFps = Statistics()->GetInfoAverageFPS();

    pStats->nWarnByte = Statistics()->GetWarnBytes();
    pStats->nWarnLine = Statistics()->GetWarnLine();
    pStats->fWarnCurrentFps = Statistics()->GetWarnCurrentFPS();
    pStats->fWarnAverageFps = Statistics()->GetWarnAverageFPS();

    pStats->nErrorByte = Statistics()->GetErrorBytes();
    pStats->nErrorLine = Statistics()->GetErrorLine();
    pStats->fErrorCurrentFps = Statistics()->GetErrorCurrentFPS();
    pStats->fErrorAverageFps = Statistics()->GetErrorAverageFPS();

    pStats->nFatalByte = Statistics()->GetFatalBytes();
    pStats->nFatalLine = Statistics()->GetFatalLine();
    pStats->fFatalCurrentFps = Statistics()->GetFatalCurrentFPS();
    pStats->fFatalAverageFps = Statistics()->GetFatalAverageFPS();

    pStats->nMainByte = Statistics()->GetMainBytes();
    pStats->nMainLine = Statistics()->GetMainLine();
    pStats->fMainCurrentFps = Statistics()->GetMainCurrentFPS();
    pStats->fMainAverageFps = Statistics()->GetMainAverageFPS();

    pStats->nRemoteByte = Statistics()->GetRemoteBytes();
    pStats->nRemoteLine = Statistics()->GetRemoteLine();
    pStats->fRemoteCurrentFps = Statistics()->GetRemoteCurrentFPS();
    pStats->fRemoteAverageFps = Statistics()->GetRemoteAverageFPS();

    pStats->nSysByte = Statistics()->GetSysBytes();
    pStats->nSysLine = Statistics()->GetSysLine();
    pStats->fSysCurrentFps = Statistics()->GetSysCurrentFPS();
    pStats->fSysAverageFps = Statistics()->GetSysAverageFPS();

    double   fTotalCurrentFps = 0;    ///< Total fps written to the log.
    double   fTotalAverageFps = 0;    ///< Total fps written to the log.

	fTotalCurrentFps += pStats->fConsoleCurrentFps;
	fTotalCurrentFps += pStats->fTraceCurrentFps;
	fTotalCurrentFps += pStats->fDebugCurrentFps;
	fTotalCurrentFps += pStats->fInfoCurrentFps;
	fTotalCurrentFps += pStats->fWarnCurrentFps;
	fTotalCurrentFps += pStats->fErrorCurrentFps;
	fTotalCurrentFps += pStats->fFatalCurrentFps;
	fTotalCurrentFps += pStats->fMainCurrentFps;
	fTotalCurrentFps += pStats->fRemoteCurrentFps;
	fTotalCurrentFps += pStats->fSysCurrentFps;

	pStats->fSysCurrentFps = fTotalCurrentFps;

    fTotalAverageFps += pStats->fConsoleAverageFps;
    fTotalAverageFps += pStats->fTraceAverageFps;
    fTotalAverageFps += pStats->fDebugAverageFps;
    fTotalAverageFps += pStats->fInfoAverageFps;
    fTotalAverageFps += pStats->fWarnAverageFps;
    fTotalAverageFps += pStats->fErrorAverageFps;
    fTotalAverageFps += pStats->fFatalAverageFps;
    fTotalAverageFps += pStats->fMainAverageFps;
    fTotalAverageFps += pStats->fRemoteAverageFps;
    fTotalAverageFps += pStats->fSysAverageFps;

    pStats->fSysCurrentFps = fTotalAverageFps;

    //////////////////////////////////////////////////////////////////////////
    pStats->nConsolePublicDQueue = Statistics()->GetConsolePublicDQueue();
    pStats->nConsolePublicTQueue = Statistics()->GetConsolePublicTQueue();
    pStats->nConsolePublicIQueue = Statistics()->GetConsolePublicIQueue();
    pStats->nConsolePublicWQueue = Statistics()->GetConsolePublicWQueue();
    pStats->nConsolePublicEQueue = Statistics()->GetConsolePublicEQueue();
    pStats->nConsolePublicFQueue = Statistics()->GetConsolePublicFQueue();
    pStats->nConsolePrivateQueue = Statistics()->GetConsolePrivateQueue();

    pStats->nTracePublicQueue    = Statistics()->GetTracePublicQueue();
    pStats->nTracePrivateQueue   = Statistics()->GetTracePrivateQueue();

    pStats->nDebugPublicQueue    = Statistics()->GetDebugPublicQueue();
    pStats->nDebugPrivateQueue   = Statistics()->GetDebugPrivateQueue();

    pStats->nInfoPublicQueue     = Statistics()->GetInfoPublicQueue();
    pStats->nInfoPrivateQueue    = Statistics()->GetInfoPrivateQueue();
                                 
    pStats->nWarnPublicQueue     = Statistics()->GetWarnPublicQueue();
    pStats->nWarnPrivateQueue    = Statistics()->GetWarnPrivateQueue();
                                 
    pStats->nErrorPublicQueue    = Statistics()->GetErrorPublicQueue();
    pStats->nErrorPrivateQueue   = Statistics()->GetErrorPrivateQueue();
                                 
    pStats->nFatalPublicQueue    = Statistics()->GetFatalPublicQueue();
    pStats->nFatalPrivateQueue   = Statistics()->GetFatalPrivateQueue();

    pStats->nMainPublicDQueue   = Statistics()->GetMainPublicDQueue();
    pStats->nMainPublicTQueue   = Statistics()->GetMainPublicTQueue();
    pStats->nMainPublicIQueue   = Statistics()->GetMainPublicIQueue();
    pStats->nMainPublicWQueue   = Statistics()->GetMainPublicWQueue();
    pStats->nMainPublicEQueue   = Statistics()->GetMainPublicEQueue();
    pStats->nMainPublicFQueue   = Statistics()->GetMainPublicFQueue();
    pStats->nMainPrivateQueue   = Statistics()->GetMainPrivateQueue();

    pStats->nRemotePublicDQueue = Statistics()->GetRemotePublicDQueue();
    pStats->nRemotePublicTQueue = Statistics()->GetRemotePublicTQueue();
    pStats->nRemotePublicIQueue = Statistics()->GetRemotePublicIQueue();
    pStats->nRemotePublicWQueue = Statistics()->GetRemotePublicWQueue();
    pStats->nRemotePublicEQueue = Statistics()->GetRemotePublicEQueue();
    pStats->nRemotePublicFQueue = Statistics()->GetRemotePublicFQueue();
    pStats->nRemotePrivateQueue = Statistics()->GetRemotePrivateQueue();

    pStats->nSysPublicFQueue   = Statistics()->GetSysPublicQueue();
    pStats->nSysPrivateQueue   = Statistics()->GetSysPrivateQueue();

    pStats->nTotalPublicQueue = 0;
    pStats->nTotalPublicQueue += pStats->nConsolePublicDQueue;
    pStats->nTotalPublicQueue += pStats->nConsolePublicTQueue;
    pStats->nTotalPublicQueue += pStats->nConsolePublicIQueue;
    pStats->nTotalPublicQueue += pStats->nConsolePublicWQueue;
    pStats->nTotalPublicQueue += pStats->nConsolePublicEQueue;
    pStats->nTotalPublicQueue += pStats->nConsolePublicFQueue;
    pStats->nTotalPublicQueue += pStats->nTracePublicQueue;
    pStats->nTotalPublicQueue += pStats->nDebugPublicQueue;
    pStats->nTotalPublicQueue += pStats->nInfoPublicQueue;
    pStats->nTotalPublicQueue += pStats->nWarnPublicQueue;
    pStats->nTotalPublicQueue += pStats->nErrorPublicQueue;
    pStats->nTotalPublicQueue += pStats->nFatalPublicQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicDQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicTQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicIQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicWQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicEQueue;
    pStats->nTotalPublicQueue += pStats->nMainPublicFQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicDQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicTQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicIQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicWQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicEQueue;
    pStats->nTotalPublicQueue += pStats->nRemotePublicFQueue;
    pStats->nTotalPublicQueue += pStats->nSysPublicFQueue;

    pStats->nTotalPrivateQueue = 0;
    pStats->nTotalPrivateQueue += pStats->nConsolePrivateQueue;

    pStats->nTotalPrivateQueue += pStats->nTracePrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nDebugPrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nInfoPrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nWarnPrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nErrorPrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nFatalPrivateQueue;

    pStats->nTotalPrivateQueue += pStats->nMainPrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nRemotePrivateQueue;
    pStats->nTotalPrivateQueue += pStats->nSysPrivateQueue;

	return true;
}

CYLOGGER_NAMESPACE_END