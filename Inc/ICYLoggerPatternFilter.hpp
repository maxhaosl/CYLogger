/*
 * CYLogger License
 * -----------
 *
 * CYLogger is licensed under the terms of the MIT license reproduced below.
 * This means that CYLogger is free software and can be used for both academic
 * and commercial purposes at absolutely no cost.
 *
 *
 * ===============================================================================
 *
 * Copyright (C) 2023-2024 ShiLiang.Hao <newhaosl@163.com>, foobra<vipgs99@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * ===============================================================================
 */

 /*
  * AUTHORS:  ShiLiang.Hao <newhaosl@163.com>, foobra<vipgs99@gmail.com>
  * VERSION:  1.0.0
  * PURPOSE:  A cross-platform efficient and stable Log library.
  * CREATION: 2023.04.15
  * LCHANGE:  2023.04.15
  * LICENSE:  Expat/MIT License, See Copyright Notice at the begin of this file.
  */

#ifndef __I_CY_LOGGER_PATTERN_FILTER_HPP__
#define __I_CY_LOGGER_PATTERN_FILTER_HPP__

#include "ICYLoggerDefine.hpp"

#include <list>
#include <assert.h>
#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

class CYLOGGER_API ICYLoggerPatternFilter
{
public:
    ICYLoggerPatternFilter() noexcept
    {
        m_plstTupleFields = new std::list<TupleFieldType>();
        assert(m_plstTupleFields);
    }

    virtual ~ICYLoggerPatternFilter() noexcept
    {
        if (m_plstTupleFields) delete m_plstTupleFields;
        m_plstTupleFields = nullptr;
    }

public:
    typedef std::pair<TString, TString> TupleFieldType;

    /**
     * @brief Add filter characters to dictionary.
    */
    ICYLoggerPatternFilter& operator<<(const TupleFieldType& tpField)
    {
        for (std::list<TupleFieldType>::iterator it = m_plstTupleFields->begin(); it != m_plstTupleFields->end(); ++it)
        {
            if (it->first == tpField.first)
            {
                if (tpField.second.empty())
                    m_plstTupleFields->erase(it);
                else
                    it->second = tpField.second;

                return *this;
            }
        }
        m_plstTupleFields->push_back(tpField);
        return *this;
    }

    /**
     * @brief Request filter character dictionary.
    */
    const std::list<TupleFieldType>& GetFilterFields()
    {
        return *m_plstTupleFields;
    }

    /**
     * @brief Request filter string.
    */
    virtual TOStringStream& FilterRequest(TOStringStream& sss, TString& strMsg, const TString& delimiters, const TChar& escapeChar, const TChar& cFiledNameEnd, const TChar& cFiledValueEnd)
    {
        for (std::list<TupleFieldType>::const_iterator it = m_plstTupleFields->begin(); it != m_plstTupleFields->end(); ++it)
        {
            sss << Escape(strMsg, it->first, delimiters, escapeChar) << cFiledNameEnd;
            sss << Escape(strMsg, it->second, delimiters, escapeChar) << cFiledValueEnd;
        }

        if (m_pNextFilter != nullptr)
        {
            return m_pNextFilter->FilterRequest(sss, strMsg, delimiters, escapeChar, cFiledNameEnd, cFiledValueEnd);
        }

        return sss;
    }

    /**
     * @brief Append filter to the end of the chain of responsibility.
    */
    virtual void SetNextFilter(ICYLoggerPatternFilter* pFilter)
    {
        if (m_pNextFilter == nullptr)
        {
            m_pNextFilter = pFilter;
        }
        else
        {
            ICYLoggerPatternFilter* pCurFilter = GetNextFilter();
            while (pCurFilter->GetNextFilter())
            {
                pCurFilter = pCurFilter->GetNextFilter();
            }
            pCurFilter->SetNextFilter(pFilter);
        }
    }

    /**
     * @brief Get the next Filter in the responsibility chain.
    */
    virtual ICYLoggerPatternFilter* GetNextFilter()
    {
        return m_pNextFilter;
    }

private:
    /**
     * @brief Special character conversion.
    */
    TString Escape(TString& strRes, const TString& strSrc, const TString& strDelimiters, TChar cEscapeChar)
    {
        if (&strRes != &strSrc)
            strRes = strSrc;
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

protected:
    ICYLoggerPatternFilter* m_pNextFilter = nullptr;
    std::list<TupleFieldType>* m_plstTupleFields = nullptr;
};

CYLOGGER_NAMESPACE_END

#endif //__I_CY_LOGGER_PATTERN_FILTER_HPP__