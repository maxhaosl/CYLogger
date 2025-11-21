#include "Common/Exception/CYExceptionLogFile.hpp"
#include "Common/SimpleLog/CYSimpleLog.hpp"

#include <assert.h>
#include <stdarg.h>

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief CYExceptionLogFile Instance.
*/
SharePtr<CYExceptionLogFile> CYExceptionLogFile::m_ptrInstance;

CYExceptionLogFile::CYExceptionLogFile()
{
	m_ptrLogFile = MakeShared<CYSimpleLogFile>();
}

CYExceptionLogFile::~CYExceptionLogFile()
{
	m_ptrLogFile.reset();
}

/**
 * @brief Get Instance.
*/
SharePtr<CYExceptionLogFile> CYExceptionLogFile::GetInstance()
{
	if (!m_ptrInstance)
	{
		m_ptrInstance = MakeShared<CYExceptionLogFile>();
	}
	return m_ptrInstance;
}

/**
 * @brief Release Instance.
*/
void CYExceptionLogFile::ReleaseInstance()
{
	m_ptrInstance.reset();
}

/**
 * @brief Init Log File.
*/
bool CYExceptionLogFile::InitLog(bool bLogTime/* = true*/, bool bLogLineCount/* = true*/, const TChar* szWorkPath/* = nullptr*/, const TChar* pszLogDir/* = LOG_DIR*/, const TChar* szFilePath/* = nullptr*/, ...)
{
	assert(m_ptrLogFile);

	va_list args;
	va_start(args, szFilePath);
	bool bRet = m_ptrLogFile->InitLog(bLogTime, bLogLineCount, szWorkPath, pszLogDir, szFilePath, args);
	va_end(args);

	return bRet;
}

/**
 * @brief Write Log.
*/
bool CYExceptionLogFile::WriteLog(const TChar* szStr, const TChar* szFile/* = __TFILE__*/, const TChar* szLocation/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/)
{
	assert(m_ptrLogFile);
	return m_ptrLogFile->WriteLog(szStr, szFile, szLocation, nLine);
}

CYLOGGER_NAMESPACE_END