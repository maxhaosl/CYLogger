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

#ifndef __CY_LOGGER_CLEAR_LOG_FILE_HPP__
#define __CY_LOGGER_CLEAR_LOG_FILE_HPP__

#include "Common/CYPrivateDefine.hpp"
#include "Entity/Schedule/CYLoggerScheduleDefine.hpp"
#include "Common/Time/CYTimeElapsed.hpp"

#include <map>
#include <list>
#include <atomic>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerClearLogFile
{
    using FileClassMap = std::map<ELogType, std::map<TString, CYLogFileInfo>>;
public:
    CYLoggerClearLogFile();
    virtual ~CYLoggerClearLogFile();

public:
    /**
     * @brief Set restriction rules
    */
    void SetRestriction(bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize);

    /**
     * @brief Clean up log processing.
    */
    void ProcessClearLog(const std::list<ELogType>& lstLogType);

private:
    /**
     * @brief Get running log file information.
    */
    void ProcessRunningLogFile(const std::list<ELogType>& lstLogType, std::list<CYLogFileInfo>& lstLogFile);

    /**
     * @brief Get log file information.
    */
    void ProcessPublicLogInfo(const std::list<ELogType>& lstLogType, std::list<TString>& lstLogFile, std::list<TString>& lstNotLogFile);

    /**
     * @brief Preprocess some data.
    */
    void PreprocessData(const std::list<CYLogFileInfo>& lstUsedLogFile, const std::list<TString>& lstLogFile, FileClassMap& mapLogFileInfo);

    /**
     * @brief Clean out all non-log files
    */
    void ProcessClearNonLog(const std::list<TString>& lstNotLogFile);

    /**
     * @brief Clean the number of log files.
    */
    void ProcessClearLogCount(FileClassMap& mapLogFileInfo);

    /**
     * @brief Clean up expired log files.
    */
    void ProcessClearLogExpired(FileClassMap& mapLogFileInfo);

    /**
     * @brief Log files of each type that exceed the specified size are cleaned.
    */
    void ProcessClearLogTypeSize(FileClassMap& mapLogFileInfo);

    /**
     * @brief Log files that exceed the specified size are cleaned.
    */
    void ProcessClearLogALLSize(FileClassMap& mapLogFileInfo);

    /**
     * @brief Get log file information.
    */
    void GetFileInfomation(CYLogFileInfo& objLogFileInfo);

    /**
     * @brief Enumerate files in the specified directory.
    */
    void EnumLogFile(TString strLogPath, std::list<TString>& vecLogList);

    /**
     * @brief Enumerate files in the specified directory.
    */
    void EnumNotLogFile(TString strLogPath, std::list<TString>& vecNotLogList);

    /**
     * @brief Determine whether the file is used.
    */
    bool IsUsedLogFile(const std::list<CYLogFileInfo>& lstUsedLogFile, const TString& strFileName);

    /**
     * @brief Get Check Name.
    */
    TString GetCheckName(const TString& strFileName);

    /**
     * @brief Get Log file type.
    */
    ELogType GetLogFileType(const std::list<CYLogFileInfo>& lstLogFile, const TString& strCheckName);

private:
    /**
     * @brief first execution.
    */
    std::atomic_bool m_bFirstProcess;

    /**
     * @brief Detect file size loss time.
    */
    CYTimeElapsed m_objElapsedCheckSizeTime;

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
};


CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_CLEAR_LOG_FILE_HPP__
