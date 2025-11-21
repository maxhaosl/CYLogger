#include "Common/Message/CYNormalMessage.hpp"
#include "Common/CYPrivateDefine.hpp"
#include "Entity/Layout/CYLoggerTemplateLayoutManager.hpp"
#include "CYCoroutine/Common/Exception/CYBaseException.hpp"
#include "Common/Exception/CYExceptionLogFile.hpp"
#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

CYNormalMessage::CYNormalMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine)
	: CYBaseMessage(strChannel, eMsgType, nServerCode, strMsg, strFile, strFunction, nLine)
{
    EXCEPTION_BEGIN
    {
#ifdef _WIN32
        m_nProcessId = GetCurrentProcessId();
#else
        m_nProcessId = getpid();
#endif
		m_ptrTemplateLayout = LoggerTemplateLayoutManager()->GetTemplateLayout();
    }
    EXCEPTION_END
}

/**
* @brief Get Type Index
*/
int32_t CYNormalMessage::GetTypeIndex()
{
    return m_ptrTemplateLayout->GetTypeIndex();
}

/**
 * @brief Get Formate Log Message.
*/
const TString CYNormalMessage::GetFormatMessage() const 
{
    CYTimeStamps objTimeStamp = GetTimeStamp();
    return m_ptrTemplateLayout->GetFormatMessage(GetChannel(), static_cast<ELogType>(GetMsgType()), GetSeverCode(), GetMsg(), GetFile(), GetFunction(), GetLine(), m_nProcessId, GetThreadId(), objTimeStamp.GetYY(), objTimeStamp.GetMM(), objTimeStamp.GetDD(), objTimeStamp.GetHR(), objTimeStamp.GetMN(), objTimeStamp.GetSC(), objTimeStamp.GetMMN(), false);
}

CYLOGGER_NAMESPACE_END