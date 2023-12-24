#include "Src/Common/CYPathConvert.hpp"
#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"
#include "Src/Common/CYPublicFunction.hpp"

#include <format>

CYLOGGER_NAMESPACE_BEGIN

TString CYPathConvert::GetLogFilePath(const TChar* szChannel, const TChar* szFile, ELogFileMode eFileMode)
{
	if (nullptr == szFile)
	{
		return TEXT("");
	}

	TString strFileALL = TEXT("");
	if (nullptr != szChannel)
	{
		strFileALL = szChannel;
		strFileALL.append(TEXT("_"));
	}
	switch (eFileMode)
	{
	case CYLOGGER_NAMESPACE::LOG_MODE_FILE_APPEND:
	{
		strFileALL.append(szFile);
		break;
	}
	case CYLOGGER_NAMESPACE::LOG_MODE_FILE_TIME:
	{
		TString strData = CYTimeStamps().GetTimeStr();
		TString strFileName = CYPublicFunction::GetFileName(szFile);
		TString strFileExt = CYPublicFunction::GetFileExt(szFile);
		strFileALL.append(std::format(TEXT("{}_{}.{}"), strFileName, strData, strFileExt));
		break;
	}
	default:
		break;
	}
	return strFileALL;
}

/**
 * @brief Convert Log Path.
*/
const TString CYPathConvert::ConvertFilePath(const TChar* pszFileName, const TChar* pszLogPath, const TChar* pszLogDir)
{
	if (pszLogDir && cy_strlen(pszLogDir) > 0)
	{
		return std::format(TEXT("{}{}{}{}"), pszLogPath, pszLogDir, LOG_SEPARATOR, pszFileName);
	}
	else
	{
		return std::format(TEXT("{}{}"), pszLogPath, pszFileName);
	}
}

/**
 * @brief Convert Log Path.
*/
const TString CYPathConvert::ConvertFilePath(const TString& strFileName, const TString& strLogPath, const TString& strLogDir/* = LOG_DIR*/)
{
	if (!strLogDir.empty())
	{
		return std::format(TEXT("{}{}{}{}"), strLogPath, strLogDir, LOG_SEPARATOR, strFileName);
	}
	else
	{
		return std::format(TEXT("{}{}"), strLogPath, strFileName);
	}
}

CYLOGGER_NAMESPACE_END