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

#ifndef __CY_LOGGER_ENTITY_HPP__
#define __CY_LOGGER_ENTITY_HPP__

#include "Inc/ICYLogger.hpp"
#include "Common/CYPrivateDefine.hpp"
#include "Entity/Appender/CYLoggerBaseAppender.hpp"
#include "Entity/Appender/CYLoggerAppenderDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

template<BaseOf APPEND> 
class CYLoggerEntity
{
public:
	CYLoggerEntity(ELogType eLogType);
	virtual ~CYLoggerEntity();

public:
	/**
	 * @brief Flush Log.
	*/
	virtual void Flush();

	/**
	 * @brief Attach Appender.
	*/
	virtual void AttachAppender(const SharePtr<APPEND>& ptrAppender);

	/**
	 * @brief Detach Appender.
	*/
	virtual void DetachAppender();

	/**
	* @brief Put message to the Log
	*/
	virtual void WriteLog(const SharePtr<CYBaseMessage>& ptrMessage);

	/**
	* @return the unique id for this appender
	*/
	virtual const ELogType GetId() const;

	/**
	 * @brief Get log file name.
	*/
	virtual const TString& GetLogName();

	/**
	 * @brief Force new log file.
	*/
	virtual void ForceNewFile();

private:
	ELogType m_eLogType;
	SharePtr<APPEND> m_ptrAppender;
};

//////////////////////////////////////////////////////////////////////////
template<BaseOf APPEND>
CYLoggerEntity<APPEND>::CYLoggerEntity(ELogType eLogType)
: m_eLogType(eLogType)
{
}

template<BaseOf APPEND>
CYLoggerEntity<APPEND>::~CYLoggerEntity()
{

}

/**
* @brief Flush Log.
*/
template<BaseOf APPEND>
void CYLoggerEntity<APPEND>::Flush()
{
	if (m_ptrAppender)
	{
		m_ptrAppender->Flush();
	}
}

/**
 * @brief Attach appender.
*/
template<BaseOf APPEND>
void CYLoggerEntity<APPEND>::AttachAppender(const SharePtr<APPEND>& ptrAppender)
{
	m_ptrAppender = ptrAppender;
}

/**
 * @brief Detach Appender.
*/
template<BaseOf APPEND>
void CYLoggerEntity<APPEND>::DetachAppender()
{
	m_ptrAppender.reset();
}

/**
* @brief Put message to the Log
*/
template<BaseOf APPEND>
void CYLoggerEntity<APPEND>::WriteLog(const SharePtr<CYBaseMessage>& ptrMessage)
{
	m_ptrAppender->WriteLog(ptrMessage);
}

/**
* @return the unique id for this appender
*/
template<BaseOf APPEND>
const ELogType CYLoggerEntity<APPEND>::GetId() const
{
	return m_ptrAppender->GetId();
}

/**
* @brief Get log file name.
*/
template<BaseOf APPEND>
const TString& CYLoggerEntity<APPEND>::GetLogName()
{
	return m_ptrAppender->GetLogName();
}

/**
 * @brief Force new log file.
*/
template<BaseOf APPEND>
void CYLoggerEntity<APPEND>::ForceNewFile()
{
	return m_ptrAppender->ForceNewFile();
}

CYLOGGER_NAMESPACE_END

//////////////////////////////////////////////////////////////////////////

#endif //__CY_LOGGER_ENTITY_HPP__