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

#ifndef __CY_LOGGER_REMOTE_APPENDER_HPP__
#define __CY_LOGGER_REMOTE_APPENDER_HPP__

#include "Entity/Appender/CYLoggerBufferAppender.hpp"
#include "Entity/Appender/CYLoggerAppenderDefine.hpp"

#include <map>

CYLOGGER_NAMESPACE_BEGIN

class CYLoggerRemoteAppender : public CYLoggerBufferAppender
{
public:
    CYLoggerRemoteAppender(const TString& strHost) noexcept;
    virtual ~CYLoggerRemoteAppender() noexcept;

public:
	/**
	* @return the unique id for this appender
	*/
	virtual const ELogType GetId() const override;

	/**
	* Outputs message on the physical device
	* @param msg message to output
	* @bFlush if true flushing may be done right after the output. It's for concrete implementation to respect this parameter
	*/
	virtual void Log(const TStringView& strMsg, int nTypeIndex, bool bFlush) override;

	/**
	* Get called when this appender become active through attaching to a CYLLoggerImpl object, and it was not active before
	*/
	virtual void OnActivate() override;

	/**
	* Attempt to Flush buffers to a physical device - default implementation does nothing
	*/
	virtual void Flush() override;

	/**
	* Returns the actual size of data on a physical device - default implementation does not respect this and throws an exception
	* @returns the actual size of data, located on a physical device
	*/
	//virtual long GetSize() {
	virtual int64_t GetSize() override;

	/**
	* Attempt to physically copy data - default implementation does not respect this and throws an exception
	*/
	virtual void Copy(const TString& strTarget) override;

	/**
	* Attempt to physically clear data - default implementation does not respect this and throws an exception
	*/
	virtual void ClearContents() override;

	/**
	 * @brief Get log file name.
	*/
	virtual const TString& GetLogName() override;

	/**
	 * @brief Force new log file.
	*/
	virtual void ForceNewFile() override;

protected:
	/**
	 * @brief Open Socket.
	*/
	void OpenSocket();

	/**
	 * @brief Close Socket.
	*/
	void CloseSocket();

	/**
	 * @brief ReOpen Socket.
	*/
	void ReOpenSocket();

	/**
	 * @brief Write Socket.
	*/
	void WriteSocket(std::string strMsg);

private:
	/**
	 * @brief message buffer.
	*/
	std::vector<char> m_buffer;

    /**
     * @brief host.
    */
    std::string m_strHost;

    /**
     * @brief Socket port.
    */
    int m_nPort;

    /**
     * @brief Remote IP.
    */
    in_addr_t m_ipAddr;

    /**
     * @brief Is Socket Init.
    */
    bool m_bSocketInit;

	/**
	 * @brief socket handle.
	*/
#ifdef	CYLOGGER_WIN_OS
    SOCKET	m_socket;
#else	
    int		m_socket;
#endif

	/**
	 * @brief Log file name.
	*/
	TString m_strLogName;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_LOGGER_REMOTE_APPENDER_HPP__