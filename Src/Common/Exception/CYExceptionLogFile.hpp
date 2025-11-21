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

#ifndef __CY_EXCEPTION_LOG_FILE_HPP__
#define __CY_EXCEPTION_LOG_FILE_HPP__

#include "Common/CYPrivateDefine.hpp"
#include "Common/Exception/CYExceptionDefine.hpp"

CYLOGGER_NAMESPACE_BEGIN

class CYSimpleLogFile;
class CYExceptionLogFile
{
public:
	/**
	 * @brief Constructor.
	*/
	CYExceptionLogFile();

	/**
	 * @brief Destructor.
	*/
	virtual ~CYExceptionLogFile();

public:
	/**
	 * @brief Get Instance.
	*/
	static SharePtr<CYExceptionLogFile> GetInstance();

	/**
	 * @brief Release Instance.
	*/
	static void ReleaseInstance();

	/**
	 * @brief Init Log File.
	*/
	bool InitLog(bool bLogTime = true, bool bLogLineCount = true, const TChar* szWorkPath = nullptr, const TChar* pszLogDir = LOG_DIR, const TChar* szFilePath = nullptr, ...);

	/**
	 * @brief Write Log.
	*/
	bool WriteLog(const TChar* szStr, const TChar* szFile = __TFILE__, const TChar* szLocation = TEXT("")/*__TFUNCTION__*/, int nLine = __TLINE__);

private:
	/**
	 * @brief Simple Log File.
	*/
	SharePtr<CYSimpleLogFile> m_ptrLogFile;

	/**
	 * @brief CYExceptionLogFile Instance.
	*/
	static SharePtr<CYExceptionLogFile> m_ptrInstance;
};

#define ExceptionLogFile() CYExceptionLogFile::GetInstance()
#define ExceptionLogFile_Free() CYExceptionLogFile::ReleaseInstance()

CYLOGGER_NAMESPACE_END

#define ExceptionLog(e) CYLOGGER_NAMESPACE::ExceptionLogFile()->WriteLog(CYCOROUTINE_NAMESPACE::AtoT(e))

#define EXCEPTION_BEGIN 	UniquePtr<CYBaseException> excp;  try {

#define EXCEPTION_END		 }catch (CYCOROUTINE_NAMESPACE::CYBaseException* e) {	excp.reset(e); ExceptionLog(excp->what()); } \
							  catch (...) { ExceptionLog("Unknown exception!"); }


#endif // __CY_EXCEPTION_LOG_FILE_HPP__