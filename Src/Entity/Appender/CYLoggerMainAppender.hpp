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


#ifndef __CY_LOGGER_TRACE_APPENDER_HPP__
#define __CY_LOGGER_TRACE_APPENDER_HPP__

#include "Src/Entity/Appender/CYLoggerBufferAppender.hpp"
#include "Src/Common/CYPublicFunction.hpp"
#include "Src/Common/CYFileRestriction.hpp"
#include "Src/Common/Thread/CYNamedLocker.hpp"

#include <map>
#include <future>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerMainAppender : public CYLoggerBufferAppender, public CYFileRestriction
{
public:
	CYLoggerMainAppender(const TString& strFileName, ELogFileMode eFileMode, ELogType eLogType);
	virtual ~CYLoggerMainAppender();

public:
	/**
	* @brief Implementation of a pure virtual method from CYLoggerBaseAppender
	*/
	const ELogType GetId() const
	{
		return m_eLogType;
	}

	/**
	* @brief Implementation of a pure virtual method from CYLoggerBaseAppender
	*/
	virtual void Log(const TStringView& strMsg, int nTypeIndex, bool bFlush) override;

	/**
	* @brief Implementation of a pure virtual method from CYLoggerBaseAppender
	*/
	void OnActivate()
	{
		if (m_objLogFile) m_objLogFile.close();
		CYPublicFunction::CreateDirectory(m_strFileName);
		OpenFile(ELogFileMode::LOG_MODE_FILE_APPEND == m_eFileMode ? TIos::out | TIos::app : TIos::out);
	}

	/**
	* @return the name of the file
	*/
	const TChar* GetFileName() const
	{
		return m_strFileName.c_str();
	}

	/**
	* @brief Inherited from CYLoggerBaseAppender
	*/
	virtual void Flush() override
	{
		if (m_objLogFile) m_objLogFile.flush();
		IfTrueThrow(m_objLogFile.fail(), TString(TEXT("Flush() failed for file ")) += m_strFileName);
	}

	/**
	* @brief Inherited from CYLoggerBaseAppender
	*/
	virtual int64_t GetSize() override
	{
		int64_t /*long*/ size = m_objLogFile.tellp();
		IfTrueThrow(m_objLogFile.fail(), TString(TEXT("tellp() failed for file ")) += m_strFileName);
		return size;
	}

	/**
	* @brief Inherited from CYLoggerBaseAppender
	*/
	virtual void Copy(const TString& strTarget) override
	{
		if (m_objLogFile) m_objLogFile.close();
		CYPublicFunction::CopyFile(m_strFileName, strTarget, false, false);
		OpenFile(TIos::out | TIos::app);
	}

	/**
	* @brief Inherited from CYLoggerBaseAppender
	*/
	virtual void ClearContents() override
	{
		if (m_objLogFile) m_objLogFile.close();
		OpenFile(TIos::out);
	}

	/**
	 * @brief Get log file name.
	*/
	virtual const TString& GetLogName() override
	{
		return m_strFileName;
	}

	/**
	 * @brief Force new log file.
	*/
	virtual void ForceNewFile() override;

private:
	/**
	 * @brief Open log file.
	*/
	void OpenFile(int flags);

	/**
	 * @brief Reopen file.
	*/
	void ReOpenFile(const TString& strNewFile);

	/**
	 * @brief Create New Log File.
	*/
	void CreateNewLogFile();

private:
	ELogType m_eLogType;
	TString m_strFileName;
	TOfStream m_objLogFile;
	ELogFileMode m_eFileMode;
	CYNamedLocker m_mutex{TEXT("CYLoggerMainAppender")};
	
	/**
	 * @brief Force new log file.
	*/
	std::atomic_bool m_bForceNewFile;

	/**
	 * @brief Promise.
	*/
	std::promise<bool> m_objNewFilePromise;
	std::future<bool>  m_objNewFileFuture;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_TRACE_APPENDER_HPP__