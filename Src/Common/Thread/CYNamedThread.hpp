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

#ifndef __CY_NAMED_THREAD_HPP__
#define __CY_NAMED_THREAD_HPP__

#include "Inc/ICYLoggerDefine.hpp"
#include <thread>
#include <string>
#include <future>

CYLOGGER_NAMESPACE_BEGIN

class CYNamedThread
{
public:
    CYNamedThread(std::string_view strName) noexcept;
    virtual ~CYNamedThread() noexcept;

public:
    virtual bool StartThread();
    virtual bool StopThread();
    virtual bool IsRunning() const noexcept;

    virtual void Run() = 0;

    std::jthread::id GetId() const noexcept;

protected:
    void Wait();
    void Entry();

    void SetThreadName(std::string_view strName) noexcept;
    static void SetName(std::string_view strName) noexcept;

private:
    std::string m_strName;
    std::jthread m_thread;

    std::stop_token m_objToken;
    std::stop_source m_objStopSource;
};

CYLOGGER_NAMESPACE_END

#endif // __CY_NAMED_THREAD_HPP__