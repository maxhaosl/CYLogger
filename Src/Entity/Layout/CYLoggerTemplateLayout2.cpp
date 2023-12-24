#include "Src/Entity/Layout/CYLoggerTemplateLayout2.hpp"
#include "Src/Entity/Filter/CYLoggerPatternFilterManager.hpp"

#include <format>

CYLOGGER_NAMESPACE_BEGIN

CYLoggerTemplateLayout2::CYLoggerTemplateLayout2()
    : ICYLoggerTemplateLayout()
    , CYLoggerTemplateLayoutEscape()
{

}

CYLoggerTemplateLayout2::~CYLoggerTemplateLayout2()
{

}


const TString CYLoggerTemplateLayout2::GetTimeStamps(int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN)
{
	return std::format(TEXT("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>3}"), nYY, nMM, nDD, nHR, nMN, nSC, nMMN);
}

/**
 * @brief Get Format Message.
 * [2023-12-23 21:11:27.599][D|P:15996|T:32572][CCYLoggerTestDlg::Entry(191)] 0 - This is a test message!
*/
TString CYLoggerTemplateLayout2::GetFormatMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine, unsigned long nProcessId, unsigned long nThreadId, int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN, bool bEscape)
{
    TChar escapeChar;
    TString delimiters;
    GetDelimiters(delimiters, escapeChar);
    TString s;
    TOStringStream ss;
    ss << delimiters[TYPE_HEADER_START] << Escape(s, GetTimeStamps(nYY, nMM, nDD, nHR, nMN, nSC, nMMN), delimiters, escapeChar, bEscape) << delimiters[TYPE_HEADER_END];
    TChar cMsgType = { 0, };

    switch (eMsgType)
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
    if (nServerCode != UNKNOWN_SEVER_CODE)
        sss << TEXT(':') << nServerCode;

    ss << delimiters[TYPE_HEADER_START] << Escape(s, sss.str(), delimiters, escapeChar, bEscape);

    // PROCESS ID.
    sss.str(TEXT(""));
    sss << TEXT("P:") << nProcessId;
    ss << delimiters[TYPE_FIELD_VALUE_END] << Escape(s, sss.str(), delimiters, escapeChar, bEscape);

    // THREAD ID
    sss.str(TEXT(""));
    sss << TEXT("T:") << nThreadId;
    ss << delimiters[TYPE_FIELD_VALUE_END] << Escape(s, sss.str(), delimiters, escapeChar, bEscape) << delimiters[TYPE_HEADER_END];

    // EXTENSION FIELD
    sss.str(TEXT(""));

    if (bEscape)
    {
        // filter chain filters custom characters.
        LoggerPatternFilterManager()->GetPatternFilter()->FilterRequest(sss, s, delimiters, escapeChar, delimiters[TYPE_FIELD_NAME_END], delimiters[TYPE_EXTENSION_FIELD_VALUE_END]);
    }

    s = sss.str();

    if (s.length() != 0)
        ss << delimiters[TYPE_HEADER_START] << s << delimiters[TYPE_HEADER_END];

    ss << delimiters[TYPE_HEADER_START]
        << Escape(s, strFunction, delimiters, escapeChar, bEscape)
        << TEXT("(")
        << nLine
        << TEXT(")")
        << delimiters[TYPE_HEADER_END]
        << TEXT(" ");

    if (!strChannel.empty())
    {
        sss.str(TEXT(""));
        sss << delimiters[TYPE_HEADER_START] << TEXT("Channel:") << strChannel << delimiters[TYPE_HEADER_END];
        ss << Escape(s, sss.str(), delimiters, escapeChar, bEscape);
    }

    ss << Escape(s, strMsg, delimiters, escapeChar, bEscape);

    return ss.str();
}

/**
 * @brief Get Type Index
*/
int32_t CYLoggerTemplateLayout2::GetTypeIndex()
{
    return 25;
}

CYLOGGER_NAMESPACE_END