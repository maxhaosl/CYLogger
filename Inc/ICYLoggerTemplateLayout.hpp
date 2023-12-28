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

#ifndef __I_CYLOGGERTEXTEMPLATE_LAYOUT_HPP__
#define __I_CYLOGGERTEXTEMPLATE_LAYOUT_HPP__

#include "ICYLoggerDefine.hpp"

#include <iomanip>
#include <sstream>

CYLOGGER_NAMESPACE_BEGIN

class CYLOGGER_API ICYLoggerTemplateLayout
{
public:
    ICYLoggerTemplateLayout()
    {
    }

    virtual ~ICYLoggerTemplateLayout()
    {
    }

public:
    /**
     * @brief Get Format Message.
    */
    virtual TString GetFormatMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine, unsigned long processId, unsigned long nThreadId, int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN, bool bEscape) = 0;

    /**
     * @brief Get Type Index
    */
    virtual int32_t GetTypeIndex() = 0;

    /**
     * @brief Get TimeStamps String.
    */
    virtual const TString GetTimeStamps(int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN)
    {
        STFormatTIme objFormatTime;
        objFormatTime.m_nYY = nYY;
        objFormatTime.m_nMM = nMM;
        objFormatTime.m_nDD = nDD;
        objFormatTime.m_nHR = nHR;
        objFormatTime.m_nMN = nMN;
        objFormatTime.m_nSC = nSC;
        objFormatTime.m_nMMN = nMMN;

        return objFormatTime.ToString();
    }

protected:
    struct STFormatTIme
    {
        int             m_nYY;      // Year
        int             m_nMM;      // Month
        int             m_nDD;      // Day
        int             m_nHR;      // Hour
        int             m_nMN;      // Minute
        int             m_nSC;      // Second
        int             m_nMMN;     // MicroSecond

        /**
         * @brief The interface does not use the C++ 20 format syntax. It uses the old setw method to format strings to prevent interface pollution. The dynamic library method does not require the project to use the C++ 20 method.
        */
        TString ToString()
        {
            TOStringStream ss;
            ss << std::setw(4) << std::setfill(TEXT('0')) << m_nYY <<
                std::setw(1) << TEXT('-') <<
                std::setw(2) << std::setfill(TEXT('0')) << m_nMM <<
                std::setw(1) << TEXT('-') <<
                std::setw(2) << std::setfill(TEXT('0')) << m_nDD <<
                std::setw(1) << TEXT(' ') <<
                std::setw(2) << std::setfill(TEXT('0')) << m_nHR <<
                std::setw(1) << TEXT(':') <<
                std::setw(2) << std::setfill(TEXT('0')) << m_nMN <<
                std::setw(1) << TEXT(':') <<
                std::setw(2) << std::setfill(TEXT('0')) << m_nSC << 
                std::setw(1) << TEXT('.') <<
                std::setw(3) << std::setfill(TEXT('0')) << m_nMMN;
           // return std::format(TEXT("{:0>4}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}.{:0>3}"), m_nYY, m_nMM, m_nDD, m_nHR, m_nMN, m_nSC, m_nMMN);
           return ss.str();
        }
    };
};

CYLOGGER_NAMESPACE_END

#endif //__I_CYLOGGERTEXTEMPLATE_LAYOUT_HPP__