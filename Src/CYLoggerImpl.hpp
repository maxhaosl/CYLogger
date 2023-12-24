#ifndef __CYLOGGER_IMPL_HPP__
#define __CYLOGGER_IMPL_HPP__

#include "Inc/ICYLogger.hpp"
#include "Src/Common/CYPrivateDefine.hpp"
#include "Src/Common/Thread/CYNamedLocker.hpp"

#include <atomic>


CYLOGGER_NAMESPACE_BEGIN

class CYLoggerControl;
class CYLoggerBaseAppender;
class CYLLoggerImpl : public ICYLogger
{
public:
	CYLLoggerImpl(ELogLevelFilter eLogFilterLevel = ELogLevelFilter::LOG_FILTER_ALL);
	virtual ~CYLLoggerImpl();

public:
	/**
	 * @brief Initialization and de-initialization of cry Log.
	*/
	virtual bool Init() noexcept override;
	virtual void UnInit() noexcept override;

	/**
	 * @brief Flush Log.
	*/
	virtual void Flush(ELogType eLogType) noexcept override;

	/**
	 * @brief Append Log file.
	*/
	virtual bool AddApender(ELogType eLogType, const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode) noexcept override;

	/**
	 * @brief Write Log.
	*/
	virtual void WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* szMsg) noexcept override;
	virtual void WriteLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) noexcept override;
	virtual void WriteEscapeLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const TChar* szMsg, ...) noexcept override;
	virtual void WriteHexLog(int nLogLevel, ELogType eMsgType, int nSeverCode, const TChar* pszFile, const TChar* pszFuncName, int nLine, const void* szMsg, int nLen) noexcept override;

	/**
	 * @brief Set Log Config.
	*/
	virtual void SetConfig(const TChar* szLogPath, bool bShowConsoleWindow) noexcept override;

    /**
     * @brief Set restriction rules
    */
	virtual void SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize) noexcept override;

    /**
     * @brief Log special character filtering.
     * @param pFilter needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetFilter(ICYLoggerPatternFilter* pFilter)  noexcept override;

    /**
     * @brief Log information format template.
     * @param pLayout needs to be saved globally, and its scope is larger than the log library instance.
    */
    virtual void SetLayout(ELogLayoutType eLayoutType, ICYLoggerTemplateLayout* pLayout)  noexcept override;

    /**
     * @brief Get statistics.
    */
	virtual bool GetStats(STStatistics* pStats) noexcept override;

	/**
	 * @brief Get Log Filter Level.
	*/
	virtual ELogLevelFilter GetLogFilterLevel() const noexcept;
	virtual void SetLogFilterLevel(ELogLevelFilter eLogFilterLevel) noexcept;

private:
	/**
	 * @brief Is Exit.
	*/
	std::atomic_bool m_bExit = false;

	/**
	 * Logger Control
	 */
	SharePtr<CYLoggerControl> m_ptrLoggerControl;
};

CYLOGGER_NAMESPACE_END

#endif // __CYLOGGER_IMPL_HPP__