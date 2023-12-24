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

#ifndef __CY_LOGGER_CONFIG_HPP__
#define __CY_LOGGER_CONFIG_HPP__

#include "Inc/ICYLogger.hpp"
#include "Src/Common/CYPrivateDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerConfig
{
public:
	/**
	 * @brief Constructor.
	*/
	CYLoggerConfig();

	/**
	 * @brief Destructor.
	*/
	virtual ~CYLoggerConfig();

public:
	/**
	 * @brief Get Custom Log Path.
	*/
	TString GetLogPath();

	/**
	 * @brief Set Custom Log Path.
	*/
	void SetLogPath(const TString& strPath);

	/**
	 * @brief Get Error Log File Name.
	*/
	TString GetErrorLogName();

	/**
	 * @brief Get Work Path.
	*/
	TString GetWorkPath();

	/**
	 * @brief Set Whether the console log displays the console window.
	*/
	void SetShowConsoleWindow(bool bShow);

	/**
	 * @brief Get Whether the console log displays the console window.
	*/
	bool GetShowConsoleWindow();

	/**
	 * @brief Set Layout Type.
	*/
	void SetLayoutType(ELogLayoutType eLayoutType);

	/**
	 * @brief Get Layout Type.
	*/
	ELogLayoutType GetLayoutType();

public:
	/**
	 * @brief Get Instance.
	*/
	static SharePtr<CYLoggerConfig> GetInstance();

	/**
	 * @brief Release Instance.
	*/
	static void ReleaseInstance();

private:
#ifdef _WIN32
	/**
	 * @brief Get windows exe path.
	*/
	TString GetExePath();
#endif

private:
	/**
	 * @brief Log layout template type.
	*/
	ELogLayoutType m_eLayoutType;

	/**
	 * @brief Error Log File Name.
	*/
	TString m_strErrLogName;

	/**
	 * @brief Custom Log Path.
	*/
	TString m_strLogPath;

	/**
	 * @brief Whether the console log displays the console window.
	*/
	bool m_bShowConsoleWindow = true;

	/**
	 * @brief Singleton Instance.
	*/
	static SharePtr<CYLoggerConfig> m_ptrInstance;
};

#define LoggerConfig() CYLoggerConfig::GetInstance()
#define FreeLoggerConfig() CYLoggerConfig::ReleaseInstance()

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_CONFIG_HPP__