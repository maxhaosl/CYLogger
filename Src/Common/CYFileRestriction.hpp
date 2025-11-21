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

#ifndef __CY_FILE_RESTRICTION_HPP__
#define __CY_FILE_RESTRICTION_HPP__

#include "Inc/ICYLoggerDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

class CYFileRestriction
{
public:
    CYFileRestriction();
    virtual ~CYFileRestriction();

public:
    /**
     * @brief Set restriction rules
    */
    static void SetRestriction(int nCheckFileSize);

    /**
     * @brief Cumulative byte size written to file.
    */
    void AddFileSize(uint64_t nSize);

    /**
     * @brief Reset file size.
    */
    void ResetFileSize();

    /**
     * @brief Get file size.
    */
    uint64_t GetFileSize();

    /**
     * @brief Whether to create new files.
    */
    bool IsCreateNewLog();

    /**
     * @brief Get new log file name.
    */
    const TString GetNewLogName(const TString& strCurrentLogName);

private:
    /**
     * @brief Detect file size limit, in bytes. If a single file exceeds this limit, it will automatically switch to a new file. This limit is only effective in the log file naming method based on date.
    */
    static int m_nCheckFileSize;

    /**
     * @brief File Size.
    */
    uint64_t m_nFileSize;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_FILE_RESTRICTION_HPP__