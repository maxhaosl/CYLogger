#include "Entity/Layout/CYLoggerTemplateLayoutCustom.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerTemplateLayoutCustom::CYLoggerTemplateLayoutCustom()
    : ICYLoggerTemplateLayout()
    , m_pCustomFilter(nullptr)
{

}

CYLoggerTemplateLayoutCustom::~CYLoggerTemplateLayoutCustom()
{

}

/**
 * @brief Get Format Message.
*/
TString CYLoggerTemplateLayoutCustom::GetFormatMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine, unsigned long nProcessId, unsigned long nThreadId, int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN, bool bEscape)
{
    if (m_pCustomFilter)
    {
        m_pCustomFilter->GetFormatMessage(strChannel, eMsgType, nServerCode, strMsg, strFile, strFunction, nLine, nProcessId, nThreadId, nYY, nMM, nDD, nHR, nMN, nSC, nMMN, bEscape);
    }
    return TEXT("");
}

/**
 * @brief Get Type Index
*/
int32_t CYLoggerTemplateLayoutCustom::GetTypeIndex()
{
    if (m_pCustomFilter)
    {
        return m_pCustomFilter->GetTypeIndex();
    }
    return 0;
}

/**
 * @brief Set Custom Layout.
*/
void CYLoggerTemplateLayoutCustom::SetCustomLayout(ICYLoggerTemplateLayout* pFilter)
{
    m_pCustomFilter = pFilter;
}

CYLOGGER_NAMESPACE_END