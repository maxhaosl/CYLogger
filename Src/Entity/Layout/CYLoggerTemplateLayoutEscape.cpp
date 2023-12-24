#include "Src/Entity/Layout/CYLoggerTemplateLayoutEscape.hpp"

CYLOGGER_NAMESPACE_BEGIN


CYLoggerTemplateLayoutEscape::CYLoggerTemplateLayoutEscape()
{
    SetDelimiters();
}

CYLoggerTemplateLayoutEscape::~CYLoggerTemplateLayoutEscape()
{

}


/**
* @brief Escape delimiters.
*/
const TChar* CYLoggerTemplateLayoutEscape::Escape(TString& strRes, const TString& strSrc, const TString& strDelimiters, TChar cEscapeChar, bool bEscape/* = true*/)
{
    if (&strRes != &strSrc)
        strRes = strSrc;

    if (!bEscape)
    {
        return strRes.c_str();
    }

    int nSize = (int)strDelimiters.size();
    for (uint32_t nIndex = 0; nIndex < strRes.size(); ++nIndex)
    {
        TChar c = strRes[nIndex];
        if (c == cEscapeChar)
        {
            strRes.insert(nIndex, 1, cEscapeChar);
            nIndex += 1;
            continue;
        }
        for (int jIndex = 0; jIndex < nSize; ++jIndex)
        {
            if (c == strDelimiters[jIndex])
            {
                strRes.insert(nIndex, 1, cEscapeChar);
                nIndex += 1;
                break;
            }
        }
    }
    return strRes.c_str();
}

CYLOGGER_NAMESPACE_END