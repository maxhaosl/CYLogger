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

#ifndef __CY_LOGGER_SCHEDULE_HPP__
#define __CY_LOGGER_SCHEDULE_HPP__

#include "Src/Common/CYPrivateDefine.hpp"
#include "Src/Common/Thread/CYNamedThread.hpp"
#include "Src/Common/Thread/CYNamedCondition.hpp"

#include <list>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerDoZipLog;
class CYLoggerClearLogFile;
class CYLoggerSchedule : public CYNamedThread
{
public:
    CYLoggerSchedule(std::string_view strName);
    virtual ~CYLoggerSchedule();

public:
    /**
     * @brief Set restriction rules
    */
    void SetRestriction(bool bEnableCheck, bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize);

    /**
     * @brief Start Schedule Thread.
    */
    void StartSchedule();

    /**
     * @brief Stop Schedule Thread.
    */
    void StopSchedule();

    /**
     * @brief Reset the log using a new log file.
    */
    void ResetLogFile();

    /**
     * @brief Add Appender Log Type.
    */
    void AddLogType(ELogType eLogType);

private:
    /**
     * @brief Thread Entry.
    */
    virtual void Run() override;

private:
    /**
     * @brief Detection switch is enabled by default.
    */
    bool m_bEnableCheck = true;

    /**
     * @brief Whether to enable packaging and uploading logs.
    */
    bool m_bEnableUpLoad = false;

    /**
     * @brief Clean up non-log files in the log directory, such as package files and log files named in non-date format that remain after switching the date naming format for log files.
    */
    bool m_bEnableClearUnLogFile = true;

    /**
     * @brief Time limit for cleaning expired log files, in seconds, default is 5 minutes.
    */
    int  m_nLimitTimeClearLog = 5 * 60;

    /**
     * @brief Limit for cleaning expired log files, in hours, default is 1 day.
    */
    int  m_nLimitTimeExpiredFile = 24;

    /**
     * @brief Time interval for detecting file size, in seconds, default is 20 seconds.
    */
    int m_nCheckFileSizeTime = 60 * 5;

    /**
     * @brief Time interval for detecting file size, in seconds, default is 20 seconds.
    */
    int m_nCheckFileCountTime = 60;

    /**
     * @brief Detect file size limit, in bytes. If a single file exceeds this limit, it will automatically switch to a new file. This limit is only effective in the log file naming method based on date.
    */
    int m_nCheckFileSize = 1024 * 1024 * 5;

    /**
     * @brief The limit on the number of log files of each type. If there are multiple limits, which limit is triggered first and which limit is calculated according to.
    */
    int m_nFileCountPerType = 20;

    /**
     * @brief The total log file size limit for each type of log. If this limit is exceeded, the log file with the earliest time and date will be cleared.
    */
    int m_nCheckFileTypeSize = 1024 * 1024 * 500;

    /**
     * @brief  The total log file size limit for all type of log. If this limit is exceeded, the log file with the earliest time and date will be cleared.
    */
    int m_nCheckALLFileSize = 1024 * 1024 * 1024;

    /**
     * @brief Rewrite new log file.
    */
    bool m_bResetLogFile = false;

	/**
	 * @brief Wait for data condition.
	*/
	CYNamedCondition m_objCondition { TEXT("CYLoggerSchedule") };

    /**
     * @brief Appender Log Type.
    */
    std::list<ELogType> m_lstLogType;

    /**
     * @brief Log package upload operation.
    */
    UniquePtr<CYLoggerDoZipLog> m_ptrDoZipLog;

    /**
     * @brief Clean log file operation.
    */
    UniquePtr<CYLoggerClearLogFile> m_ptrClearLogFile;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_SCHEDULE_HPP__