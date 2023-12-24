#include "Src/Common/Message/CYEscapeMessage.hpp"
#include "Src/Common/CYPrivateDefine.hpp"
#include "Src/Entity/Filter/CYLoggerPatternFilterManager.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayoutManager.hpp"
#include "CYCoroutine/Common/Exception/CYBaseException.hpp"
#include "Src/Common/Exception/CYExceptionLogFile.hpp"

#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

CYEscapeMessage::CYEscapeMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine) 
: CYBaseMessage(strChannel, eMsgType, nServerCode, strMsg, strFile, strFunction, nLine)
{
	EXCEPTION_BEGIN
	{
        m_nProcessId = GetCurrentProcessId();
		m_ptrTemplateLayout = LoggerTemplateLayoutManager()->GetTemplateLayout();
	}
	EXCEPTION_END
}

/**
* @brief Get Type Index
*/
int32_t CYEscapeMessage::GetTypeIndex()
{
	return m_ptrTemplateLayout->GetTypeIndex();
}

/**
 * @brief Get Formate Log Message.
*/
const TString CYEscapeMessage::GetFormatMessage() const
{
	CYTimeStamps objTimeStamp = GetTimeStamp();
	return m_ptrTemplateLayout->GetFormatMessage(GetChannel(), static_cast<ELogType>(GetMsgType()), GetSeverCode(), GetMsg(), GetFile(), GetFunction(), GetLine(), m_nProcessId, GetThreadId(), objTimeStamp.GetYY(),objTimeStamp.GetMM(), objTimeStamp.GetDD(), objTimeStamp.GetHR(), objTimeStamp.GetMN(), objTimeStamp.GetSC(), objTimeStamp.GetMMN(), true);
}

CYLOGGER_NAMESPACE_END