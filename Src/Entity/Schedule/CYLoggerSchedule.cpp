#include "Src/Entity/Schedule/CYLoggerSchedule.hpp"
#include "Src/Entity/Schedule/CYLoggerDoZipLog.hpp"
#include "Src/Entity/Schedule/CYLoggerClearLogFile.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "Src/Common/Exception/CYExceptionLogFile.hpp"
#include "Src/Common/Time/CYTimeElapsed.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerSchedule::CYLoggerSchedule(std::string_view strName)
    : CYNamedThread(strName)
{
    m_ptrDoZipLog = MakeUnique<CYLoggerDoZipLog>();
    m_ptrClearLogFile = MakeUnique<CYLoggerClearLogFile>();

    IfTrueThrow(nullptr == m_ptrDoZipLog, TEXT("m_ptrDoZipLog cannot be empty."));
    IfTrueThrow(nullptr == m_ptrClearLogFile, TEXT("m_ptrClearLogFile cannot be empty."));
}

CYLoggerSchedule::~CYLoggerSchedule()
{
    m_ptrDoZipLog.reset();
    m_ptrClearLogFile.reset();
}

/**
 * @brief Set restriction rules
*/
void CYLoggerSchedule::SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize)
{
    m_bEnableCheck = bEnableCheck;
    m_bEnableClearUnLogFile = m_bClearUnLogFile;
    m_nLimitTimeClearLog = nLimitTimeClearLog;
    m_nLimitTimeExpiredFile = nLimitTimeExpiredFile;
    m_nCheckFileSizeTime = nCheckFileSizeTime;
    m_nCheckFileCountTime = nCheckFileCountTime;
    m_nCheckFileSize = nCheckFileSize;
    m_nFileCountPerType = nFileCountPerType;
    m_nCheckFileTypeSize = nCheckFileTypeSize;
    m_nCheckALLFileSize = nCheckALLFileSize;
}

/**
 * @brief Start Schedule Thread.
*/
void CYLoggerSchedule::StartSchedule()
{
    CYNamedThread::StartThread();
}

/**
 * @brief Stop Schedule Thread.
*/
void CYLoggerSchedule::StopSchedule()
{
    CYNamedThread::StopThread();
}

/**
 * @brief Reset the log using a new log file.
*/
void CYLoggerSchedule::ResetLogFile()
{
    m_bResetLogFile = true;
}

/**
 * @brief Add Appender Log Type.
*/
void CYLoggerSchedule::AddLogType(ELogType eLogType)
{
    m_lstLogType.push_back(eLogType);
}

/**
 * @brief Thread Entry.
*/
void CYLoggerSchedule::Run()
{
    IfTrueThrow(nullptr == m_ptrDoZipLog, TEXT("m_ptrDoZipLog cannot be empty."));
    IfTrueThrow(nullptr == m_ptrClearLogFile, TEXT("m_ptrClearLogFile cannot be empty."));

    m_ptrDoZipLog->SetConfig();
    m_ptrClearLogFile->SetRestriction(m_bEnableClearUnLogFile, m_nLimitTimeClearLog, m_nLimitTimeExpiredFile, m_nCheckFileSizeTime, m_nCheckFileCountTime, m_nCheckFileSize, m_nFileCountPerType, m_nCheckFileTypeSize, m_nCheckALLFileSize);

    if (m_bEnableCheck)
    {
        m_ptrClearLogFile->ProcessClearLog(m_lstLogType);
    }

    if (m_bEnableUpLoad)
    {
        m_ptrDoZipLog->Process();
        m_bEnableUpLoad = false;
    }

    CYTimeElapsed objDetectionTime;     // Detection interval.
    while (CYNamedThread::IsRunning())
    {
        EXCEPTION_BEGIN
        {
            // Upload Log File.
            if (m_bEnableUpLoad)
            {
                m_ptrDoZipLog->Process();
                m_bEnableUpLoad = false;
            }

            // Timeout detection.
            if (m_objCondition.WaitForEvent(m_nLimitTimeClearLog * 1000) != RetCode::ERR_COND_TIMEOUT)
                continue;

            // Detection switch is off.
            if (!m_bEnableCheck)
                continue;

            // Thread stopped.
            if (!CYNamedThread::IsRunning()) break;

            // Detection Log File.
            if (objDetectionTime.Elapsed() >= m_nLimitTimeClearLog)
            {
                // Clear Log File.
                m_ptrClearLogFile->ProcessClearLog(m_lstLogType);

                objDetectionTime.Reset();
            }
        }
        EXCEPTION_END
    }
}

CYLOGGER_NAMESPACE_END