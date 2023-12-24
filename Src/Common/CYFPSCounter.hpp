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

#ifndef __CY_FPS_COUNTER_HPP__
#define __CY_FPS_COUNTER_HPP__

#include "Inc/ICYLoggerDefine.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <stdexcept>

CYLOGGER_NAMESPACE_BEGIN

class CYFPSCounter
{
public:
    CYFPSCounter(int nMeasureDuration);
    virtual ~CYFPSCounter() noexcept = default;

    void StartCounter();
    void UpdateCounter();

    double GetCurrentFPS() const;
    double GetAverageFPS() const;

private:
    void UpdateAverageFPS(int nIndex);

private:
    int m_nFrameCount[2]; 
    int m_nMeasureDuration;

    double m_fCurrentFps;
    double m_fAverageFps;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_tpStartTime0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_tpStartTime1;
};

CYLOGGER_NAMESPACE_END

#endif //__CY_FPS_COUNTER_HPP__
