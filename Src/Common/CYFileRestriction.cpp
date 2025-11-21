#include "Common/CYFileRestriction.hpp"
#include "Common/CYPublicFunction.hpp"
#include "Common/CYFormatDefine.hpp"
#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief Detect file size limit, in bytes. If a single file exceeds this limit, it will automatically switch to a new file. This limit is only effective in the log file naming method based on date.
*/
int CYFileRestriction::m_nCheckFileSize = 1024 * 1024 * 5;

CYFileRestriction::CYFileRestriction()
    : m_nFileSize(0)
{
}

CYFileRestriction::~CYFileRestriction()
{
}

/**
 * @brief Set restriction rules
*/
void CYFileRestriction::SetRestriction(int nCheckFileSize)
{
    m_nCheckFileSize = nCheckFileSize;
}

void CYFileRestriction::AddFileSize(uint64_t nSize)
{
    m_nFileSize += nSize;
}

/**
 * @brief Reset file size.
*/
void CYFileRestriction::ResetFileSize()
{
    m_nFileSize = 0;
}

/**
 * @brief Get file size.
*/
uint64_t CYFileRestriction::GetFileSize()
{
    return m_nFileSize;
}

/**
 * @brief Whether to create new files.
*/
bool CYFileRestriction::IsCreateNewLog()
{
    return m_nFileSize > m_nCheckFileSize;
}

/**
 * @brief Get new log file name.
*/
const TString CYFileRestriction::GetNewLogName(const TString& strCurrentLogName)
{
    TString strData = CYTimeStamps().GetTimeStr();
    TString strFilePath = CYPublicFunction::GetBasePath(strCurrentLogName);
    TString strFileName = CYPublicFunction::GetBaseLogName(strCurrentLogName);

    TString strFileExt = CYPublicFunction::GetFileExt(strCurrentLogName);
    return strFilePath + TEXT("_") + strData + TEXT(".") + strFileExt;
}

CYLOGGER_NAMESPACE_END