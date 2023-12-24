#ifndef __CY_LOGGER_CONTROL_HPP__
#define __CY_LOGGER_CONTROL_HPP__

#include "Inc/ICYLogger.hpp"
#include "Src/Common/CYPrivateDefine.hpp"
#include "Src/Entity/CYLoggerEntity.hpp"
#include "CYCoroutine/Common/Message/CYBaseMessage.hpp"

#include <array>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerSchedule;
class CYLoggerBaseAppender;
class CYLoggerControl
{
public:
	CYLoggerControl(ELogLevelFilter eLogFilterLevel = ELogLevelFilter::LOG_FILTER_ALL);
	virtual ~CYLoggerControl();

public:
	/**
	 * @brief Initialization and de-initialization of cry Log.
	*/
	virtual bool Init();
	virtual void UnInit();

	/**
	 * @brief Flush Log.
	*/
	virtual void Flush(ELogType eLogType);

	/**
	 * @brief Append Log file.
	*/
	virtual bool AddApender(ELogType eLogType, const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode);

	/**
	* @brief Put message to the Log
	*/
	virtual void WriteLog(int nLogLevel, const SharePtr<CYBaseMessage>& ptrMessage);

	/**
	 * @brief Get Log Filter Level.
	*/
	virtual ELogLevelFilter GetLogFilterLevel() const;
	virtual void SetLogFilterLevel(ELogLevelFilter eLogFilterLevel);

    /**
     * @brief Set restriction rules
    */
	virtual void SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize);

    /**
     * @brief Log special character filtering.
     * @param pFilter needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetFilter(ICYLoggerPatternFilter* pFilter);

    /**
     * @brief Log information format template.
     * @param pLayout needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetLayout(ICYLoggerTemplateLayout* pLayout);

    /**
     * @brief Get statistics.
    */
	virtual bool GetStats(STStatistics* pStats);

private:
	/**
	 * @brief Log cleaning and other scheduling.
	*/
	UniquePtr<CYLoggerSchedule> m_ptrSchedule;

	/**
	 * @brief Log Filter Level.
	*/
	ELogLevelFilter m_eLogFilterLevel = ELogLevelFilter::LOG_FILTER_ALL;

	/**
	 * @brief Array of Entity.
	*/
	std::array<SharePtr<CYLoggerEntity<CYLoggerBaseAppender>>, ELogType::LOG_TYPE_MAX> m_arrayEntity;
};

CYLOGGER_NAMESPACE_END

#endif // __CY_LOGGER_CONTROL_HPP__