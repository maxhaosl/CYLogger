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

#ifndef __CY_LOGGER_ENTITY_FACTORY_HPP__
#define __CY_LOGGER_ENTITY_FACTORY_HPP__

#include "Inc/ICYLogger.hpp"
#include "Src/Common/CYPrivateDefine.hpp"
#include "Src/Entity/Appender/CYLoggerAppenderDefine.hpp"

#include <map>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerBaseAppender;
template<BaseOf APPEND> class CYLoggerEntity;
class CYLoggerEntityFactory
{
public:
	CYLoggerEntityFactory();
	virtual ~CYLoggerEntityFactory();

public:
	/**
	 * @brief Create Entity.
	*/
	static SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> CreateEntity(ELogType eLogType, const TString& strFileName, ELogFileMode eFileMode);

    /**
     * @brief Get Logger Entity.
    */
    SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> GetLoggerEntity(ELogType eLogType);

    /**
     * @brief Release Logger Entity.
    */
    void ReleaseLoggerEntity(ELogType eLogType);

    /**
     * @brief Release All Logger Entity.
    */
    void ReleaseAllLoggerEntity();

    /**
	 * @brief Force new log file.
	*/
	void ForceEntityNewFile();

    /**
     * @brief Get CYLoggerEntityFactory Instance.
    */
    static std::shared_ptr<CYLoggerEntityFactory> GetInstance();

    /**
     * @brief Release CYLoggerEntityFactory Instance.
    */
    static void ReleaseInstance();

private:
    //
    // Register Logger Entity.
    //
    static SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> RegisterLoggerEntity(ELogType eLogType, const TString& strFileName, ELogFileMode eFileMode);

private:
	/**
	 * @brief Register Entity Map.
	*/
	using RegisterEntity = std::map<ELogType, SharePtr<CYLoggerEntity<CYLoggerBaseAppender>>>;

	/**
	 * @brief Map of Logger Entity.
	*/
	static  RegisterEntity m_mapRegisterEntity;

    /**
     * @brief Sington.
    */
    static SharePtr<CYLoggerEntityFactory> m_ptrInstance;
};

#define LoggerEntityFactory() CYLoggerEntityFactory::GetInstance()
#define LoggerEntityFactory_Free() CYLoggerEntityFactory::ReleaseInstance()

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_ENTITY_FACTORY_HPP__