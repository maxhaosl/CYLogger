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

#ifndef __CY_LOGGER_TEMPLATE_LAYOUT_1_HPP__
#define __CY_LOGGER_TEMPLATE_LAYOUT_1_HPP__

#include "Inc/ICYLoggerDefine.hpp"
#include "Inc/ICYLoggerTemplateLayout.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayoutEscape.hpp"

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerTemplateLayout1: public ICYLoggerTemplateLayout, public CYLoggerTemplateLayoutEscape
{
public:
    CYLoggerTemplateLayout1();
    virtual ~CYLoggerTemplateLayout1();

public:
    /**
     * @brief Get Format Message.
    */
    virtual TString GetFormatMessage(const TString& strChannel, ELogType eMsgType, int nServerCode, const TString& strMsg, const TString& strFile, const TString& strFunction, int nLine, unsigned long nProcessId, unsigned long nThreadId, int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN, bool bEscape) override;

    /**
     * @brief Get Type Index
    */
    virtual int32_t GetTypeIndex() override;

	/**
	 * @brief Get Log TimeStamps.
	*/
	virtual const TString GetTimeStamps(int nYY, int nMM, int nDD, int nHR, int nMN, int nSC, int nMMN) override;
};


CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_TEMPLATE_LAYOUT_1_HPP__