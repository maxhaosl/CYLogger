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

#ifndef __CY_NAMED_CONDITION_HPP__
#define __CY_NAMED_CONDITION_HPP__

#include "Inc/ICYLoggerDefine.hpp"
#include "Src/Common/CYNoCopy.hpp"
#include "Src/Common/CYPrivateDefine.hpp"

#include <mutex>
#include <functional>

CYLOGGER_NAMESPACE_BEGIN

using FunPred = bool (*)();

class CYNamedCondition : public CYNoCopy
{
public:
	/**
	 * @brief Constructor.
	 */
	CYNamedCondition(const TChar* pszName, bool bWait = true);

	/**
	 * @brief Destructor.
	 */
	virtual ~CYNamedCondition() noexcept;

public:
	/**
	 * @brief Operation waiting.
	*/
	virtual RetCode WaitForEvent();

	/**
	 * @brief Operation waiting for timeout.
	*/
	virtual RetCode WaitForEvent(unsigned long nTimeoutMilliseconds);

	/**
	 * @brief Operation waiting for timeout.
	*/
	virtual RetCode WaitForEvent(unsigned long nTimeoutMilliseconds, std::function<bool()>&& pred);

	/**
	 * @brief Notify an execution.
	*/
	virtual RetCode SignalOne(bool bWait = false);

	/**
	 * @brief Notify all executions.
	*/
	virtual RetCode SignalAll(bool bWait = false);

	/**
	 * @brief Reset Condition.
	*/
	virtual RetCode Reset();

private:
	bool					m_bWait = true;
	TChar					m_szName[512] = {0};
	std::mutex				m_mutex;
	std::condition_variable m_cvCondition;
};



CYLOGGER_NAMESPACE_END

#endif //__CY_NAMED_CONDITION_HPP__