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

#ifndef __CY_LOGGER_BUFFER_APPENDER_HPP__
#define __CY_LOGGER_BUFFER_APPENDER_HPP__

#include "Entity/Appender/CYLoggerBaseAppender.hpp"

#include <string>
#include <map>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerBufferAppender : public CYLoggerBaseAppender
{
public:
	/**
	 * @brief Constructor.
	*/
	CYLoggerBufferAppender(std::string_view strName) noexcept;

	/**
	 * @brief Destructor.
	*/
	virtual ~CYLoggerBufferAppender() noexcept;

public:
    /**
    * @brief Put message to the Log
    */
	virtual void WriteLog(const SharePtr<CYBaseMessage>& ptrMessage) override;

	/**
	 * @brief Flip buffering.
	*/
	virtual void FlipBuffer() override;

protected:
	/**
	 * @brief Coroutine work function.
	*/
	CYCOROUTINE_NAMESPACE::CYResult<int> DoFlipBuffer(std::function<int()>&& fun);

	/**
	 * @brief Update Public Statistics.
	*/
	virtual void UpdatePublicStats();

	/**
	 * @brief Update Private Statistics.
	*/
	virtual void UpdatePrivateStats();

private:
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateDebugMessage;
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateTraceMessage;
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateInfoMessage;
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateWarnMessage;
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateErrMessage;
    std::deque<SharePtr<CYBaseMessage>> m_lstPrivateFatalMessage;
    std::multimap<int64_t, SharePtr<CYBaseMessage>> m_mapPrivateMessage;

    CYNamedLocker m_mutexPublicDebugMessage{ TEXT("Trace_PublicDebugMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicDebugMessage;

    CYNamedLocker m_mutexPublicTraceMessage{ TEXT("Trace_PublicTraceMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicTraceMessage;

    CYNamedLocker m_mutexPublicInfoMessage{ TEXT("Trace_PublicInfoMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicInfoMessage;

    CYNamedLocker m_mutexPublicWarnMessage{ TEXT("Trace_PublicWarnMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicWarnMessage;

    CYNamedLocker m_mutexPublicErrMessage{ TEXT("Trace_PublicErrMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicErrMessage;

    CYNamedLocker m_mutexPublicFatalMessage{ TEXT("Trace_PublicFatalMessage") };
    std::deque<SharePtr<CYBaseMessage>> m_lstPublicFatalMessage;
};


CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_BUFFER_APPENDER_HPP__