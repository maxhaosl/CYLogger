#include "Common/CYPathConvert.hpp"
#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"
#include "Common/CYPublicFunction.hpp"
#include "Common/CYFormatDefine.hpp"

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
        strFileALL.append(strFileName + strData + TEXT(".") + strFileExt);
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
#ifdef CYLOGGER_WIN_OS
    if (pszLogDir && cy_strlen(pszLogDir) > 0)
    {
        return fmtx::format(TEXT("{}{}{}{}"), pszLogPath, pszLogDir, LOG_SEPARATOR, pszFileName);
    }
    else
    {
        return fmtx::format(TEXT("{}{}"), pszLogPath, pszFileName);
    }
#else
    if (pszLogDir && cy_strlen(pszLogDir) > 0)
    {
        return TString(pszLogPath) + TString(pszLogDir) + LOG_SEPARATOR + TString(pszFileName);
    }
    else
    {
        return TString(pszLogPath) + TString(pszFileName);
    }
#endif
}

/**
 * @brief Convert Log Path.
*/
const TString CYPathConvert::ConvertFilePath(const TString& strFileName, const TString& strLogPath, const TString& strLogDir/* = LOG_DIR*/)
{
#ifdef CYLOGGER_WIN_OS
    if (!strLogDir.empty())
    {
        return fmtx::format(TEXT("{}{}{}{}"), strLogPath, strLogDir, LOG_SEPARATOR, strFileName);
    }
    else
    {
        return fmtx::format(TEXT("{}{}"), strLogPath, strFileName);
    }
#else
    if (!strLogDir.empty())
    {
        return strLogPath + strLogDir + LOG_SEPARATOR + strFileName;
    }
    else
    {
        return strLogPath + strFileName;
    }
#endif
}

CYLOGGER_NAMESPACE_END