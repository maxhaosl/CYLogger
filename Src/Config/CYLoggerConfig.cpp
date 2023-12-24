#include "Src/Config/CYLoggerConfig.hpp"
#include "Src/Common/CYPublicFunction.hpp"

#include <iostream>
#include <cstdlib>

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief Singleton Instance.
*/
SharePtr<CYLoggerConfig> CYLoggerConfig::m_ptrInstance;

CYLoggerConfig::CYLoggerConfig()
{
	m_strErrLogName = TEXT("Exception.log");
}

CYLoggerConfig::~CYLoggerConfig()
{

}

TString CYLoggerConfig::GetLogPath()
{
	return m_strLogPath;
}

void CYLoggerConfig::SetLogPath(const TString& strPath)
{
	m_strLogPath = strPath;
}

TString CYLoggerConfig::GetErrorLogName()
{
	return m_strErrLogName;
}

TString CYLoggerConfig::GetWorkPath()
{
#ifdef _WIN32
	return GetExePath();
#else
	TString strPath = std::getenv("PWD");
	strPath += "/";
	return strPath;
#endif
}

/**
 * @brief Get Instance.
*/
SharePtr<CYLoggerConfig> CYLoggerConfig::GetInstance()
{
	if (!m_ptrInstance)
	{
		m_ptrInstance = MakeShared<CYLoggerConfig>();
	}
	return m_ptrInstance;
}	

/**
 * @brief Release Instance.
*/
void CYLoggerConfig::ReleaseInstance()
{
	m_ptrInstance.reset();
}

#ifdef _WIN32
/**
 * @brief Get windows exe path.
*/
TString CYLoggerConfig::GetExePath()
{
	TChar szAppPath[512] = { 0 };
	TChar szFileName[_MAX_PATH];
	::GetModuleFileName(nullptr, szFileName, _MAX_PATH);

	TChar* dot = cy_csrchr(szFileName, '\\');
	::lstrcpyn(szAppPath, szFileName, (int)(dot - szFileName + 2));
	return szAppPath;
}
#endif

/**
	* @brief Set Whether the console log displays the console window.
*/
void CYLoggerConfig::SetShowConsoleWindow(bool bShow)
{
	m_bShowConsoleWindow = bShow;
}

/**
	* @brief Get Whether the console log displays the console window.
*/
bool CYLoggerConfig::GetShowConsoleWindow()
{
	return m_bShowConsoleWindow;
}

/**
 * @brief Set Layout Type.
*/
void CYLoggerConfig::SetLayoutType(ELogLayoutType eLayoutType)
{
	m_eLayoutType = eLayoutType;
}

/**
 * @brief Get Layout Type.
*/
ELogLayoutType CYLoggerConfig::GetLayoutType()
{
	return m_eLayoutType;
}

CYLOGGER_NAMESPACE_END