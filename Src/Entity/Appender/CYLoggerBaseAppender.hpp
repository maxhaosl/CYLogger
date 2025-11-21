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

#ifndef __CY_LOGGER_BASE_APPENDER_HPP__
#define __CY_LOGGER_BASE_APPENDER_HPP__

#include "Common/Thread/CYNamedThread.hpp"
#include "Common/Thread/CYNamedLocker.hpp"
#include "Common/Thread/CYNamedCondition.hpp"
#include "Common/Time/CYTimeElapsed.hpp"
#include "Common/CYFPSCounter.hpp"

#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "CYCoroutine/Results/CYResult.hpp"

#include <deque>
#include <mutex>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerBaseAppender : public CYNamedThread, public CYNoCopy, public CYFPSCounter
{
public:
	/**
	 * @brief Constructor.
	*/
	CYLoggerBaseAppender(std::string_view strName) noexcept;

	/**
	 * @brief Destructor.
	*/
	virtual ~CYLoggerBaseAppender() noexcept;

	/**
	* @return the unique id for this appender
	*/
	virtual const ELogType GetId() const = 0;

	/**
	* Outputs message on the physical device
	* @param msg message to output
	* @bFlush if true flushing may be done right after the output. It's for concrete implementation to respect this parameter
	*/
	virtual void Log(const TStringView& strMsg, int nTypeIndex, bool bFlush) = 0;

	/**
	* Get called when this appender become active through attaching to a CYLLoggerImpl object, and it was not active before
	*/
	virtual void OnActivate() = 0;

	/**
	* Attempt to Flush buffers to a physical device - default implementation does nothing
	*/
	virtual void Flush() = 0;

	/**
	 * @brief Get log file name.
	*/
	virtual const TString& GetLogName() = 0;

	/**
	 * @brief Force new log file.
	*/
	virtual void ForceNewFile() = 0;

	/**
	* Returns the actual size of data on a physical device - default implementation does not respect this and throws an exception
	* @returns the actual size of data, located on a physical device
	*/
	//virtual long GetSize() {
	virtual int64_t GetSize()
	{
		IfTrueThrow(true, TEXT("operation not supported"));
		return 0;
	}

	/**
	* Attempt to physically copy data - default implementation does not respect this and throws an exception
	*/
	virtual void Copy(const TString& strTarget)
	{
		IfTrueThrow(true, TEXT("operation not supported"));
	}

	/**
	* Attempt to physically clear data - default implementation does not respect this and throws an exception
	*/
	virtual void ClearContents()
	{
		IfTrueThrow(true, TEXT("operation not supported"));
	}

	/**
	* @brief Put message to the Log
	*/
	virtual void WriteLog(const SharePtr<CYBaseMessage>& ptrMessage)
	{
        LockGuard locker(m_mutexPublicMessage);
        m_lstPublicMessage.push_back(ptrMessage);

		UpdatePublicStats();
	}

	/**
	 * @brief Flip buffering.
	*/
	virtual void FlipBuffer();

protected:
	/**
	 * @brief Start Log Thread.
	*/
	void StartLogThread();

	/**
	 * @brief Stop Log Thread.
	*/
	void StopLogThread();

	/**
	 * @brief Run function of NamedThread.
	*/
	void Run() override;

	/**
	 * @brief Update Public Statistics.
	*/
	virtual void UpdatePublicStats();

	/**
	 * @brief Update Private Statistics.
	*/
	virtual void UpdatePrivateStats();

	/**
	 * @brief Coroutine work function.
	*/
	CYCOROUTINE_NAMESPACE::CYResult<std::tuple<int, int>> DoWork(std::function<std::tuple<int, int>()>&& fun);

protected:
	/**
	 * @brief Private mutex.
	*/
	CYNamedLocker m_mutexPublicMessage{ TEXT("CYLoggerBaseAppender") };

	/**
	 * @brief Private message list.
	*/
	std::deque<SharePtr<CYBaseMessage>> m_lstPublicMessage;


	bool m_bActivate = false;

	/**
	 * @brief Wait for data condition.
	*/
	CYNamedCondition m_objCondition { TEXT("CYLoggerBaseAppender") };

	/**
	 * @brief Private message list.
	*/
	std::deque<SharePtr<CYBaseMessage>> m_lstPrivMessage;

	/**
	 * @brief Record log processing time.
	*/
	CYTimeElapsed m_objTimeElapsed;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_BASE_APPENDER_HPP__