#include "Src/Common/Message/CYStrMessage.hpp"
#include "Src/Common/CYPrivateDefine.hpp"

#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

CYStrMessage::CYStrMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg)
: CYBaseMessage(strChannel, eMsgType, nServerCode, strMsg)
, CYLoggerTemplateLayoutEscape()
{
	processId = GetCurrentProcessId();
}

/**
* @brief Get Type Index
*/
int32_t CYStrMessage::GetTypeIndex()
{
    return 0;
}

/**
 * @brief Get Formate Log Message.
*/
const TString CYStrMessage::GetFormatMessage() const 
{
	TChar escapeChar;
	TString delimiters;
	GetDelimiters(delimiters, escapeChar);
	TString s;
	TOStringStream ss;
	ss << delimiters[TYPE_HEADER_START] << GetTimeStamp().ToString();
	TChar cMsgType = { 0, };

    switch (GetMsgType())
    {
    case LOG_TYPE_DEBUG:
        cMsgType = TEXT('D');
        break;
    case LOG_TYPE_TRACE:
        cMsgType = TEXT('T');
        break;
    case LOG_TYPE_INFO:
        cMsgType = TEXT('I');
        break;
    case LOG_TYPE_WARN:
        cMsgType = TEXT('W');
        break;
    case LOG_TYPE_ERROR:
        cMsgType = TEXT('E');
        break;
    case LOG_TYPE_FATAL:
        cMsgType = TEXT('F');
        break;
    default:
        cMsgType = TEXT('U');
        break;
    };

	TOStringStream sss;
	sss << cMsgType;
	int nServerCode = GetSeverCode();
	if (nServerCode != UNKNOWN_SEVER_CODE)
		sss << TEXT(':') << nServerCode;

	ss << delimiters[TYPE_FIELD_VALUE_END] << sss.str();

	// PROCESS ID.
	sss.str(TEXT(""));
	sss << TEXT("P:") << processId;
	ss << delimiters[TYPE_FIELD_VALUE_END] << sss.str();

	// THREAD ID
	sss.str(TEXT(""));
	sss << TEXT("T:") << GetThreadId();
	ss << delimiters[TYPE_FIELD_VALUE_END] << sss.str();

	ss << delimiters[TYPE_HEADER_END]
	   << TEXT(" ");

	ss << GetMsg();

	return ss.str();
}

CYLOGGER_NAMESPACE_END