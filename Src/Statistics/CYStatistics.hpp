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

#ifndef __CY_STATISTICS_HPP__
#define __CY_STATISTICS_HPP__

#include "Inc/ICYLoggerDefine.hpp"

#include <atomic>

CYLOGGER_NAMESPACE_BEGIN

class CYStatistics final
{
public:
    CYStatistics() noexcept = default;
    virtual ~CYStatistics() noexcept = default;

public:
    void AddTotalLine(uint64_t nLine);
    uint64_t GetTotalLine();

    void AddTotalBytes(uint64_t nBytes);
    uint64_t GetTotalBytes();

    void AddTotalCurrentFPS(double fps);
    double GetTotalCurrentFPS();

    void AddTotalAverageFPS(double fps);
    double GetTotalAverageFPS();

    void AddTotalPublicQueue(uint32_t nSize);
    uint32_t GetTotalPublicQueue();

    void AddTotalPrivateQueue(uint32_t nSize);
    uint32_t GetTotalPrivateQueue();

    void AddConsoleLine(uint64_t nLine);
    uint64_t GetConsoleLine();

    void AddConsoleBytes(uint64_t nBytes);
    uint64_t GetConsoleBytes();

    void AddConsoleCurrentFPS(double fps);
    double GetConsoleCurrentFPS();

    void AddConsoleAverageFPS(double fps);
    double GetConsoleAverageFPS();

    void AddConsolePublicDQueue(uint32_t nSize);
    void AddConsolePublicTQueue(uint32_t nSize);
    void AddConsolePublicIQueue(uint32_t nSize);
    void AddConsolePublicWQueue(uint32_t nSize);
    void AddConsolePublicEQueue(uint32_t nSize);
    void AddConsolePublicFQueue(uint32_t nSize);
    void AddConsolePrivateQueue(uint32_t nSize);

    uint32_t GetConsolePublicDQueue();
    uint32_t GetConsolePublicTQueue();
    uint32_t GetConsolePublicIQueue();
    uint32_t GetConsolePublicWQueue();
    uint32_t GetConsolePublicEQueue();
    uint32_t GetConsolePublicFQueue();
    uint32_t GetConsolePrivateQueue();

    void AddTraceLine(uint64_t nLine);
    uint64_t GetTraceLine();

    void AddTraceBytes(uint64_t nBytes);
    uint64_t GetTraceBytes();

    void AddTraceCurrentFPS(double fps);
    double GetTraceCurrentFPS();

    void AddTraceAverageFPS(double fps);
    double GetTraceAverageFPS();

    void AddTracePublicQueue(uint32_t nSize);
    uint32_t GetTracePublicQueue();

    void AddTracePrivateQueue(uint32_t nSize);
    uint32_t GetTracePrivateQueue();

    void AddDebugLine(uint64_t nLine);
    uint64_t GetDebugLine();

    void AddDebugBytes(uint64_t nBytes);
    uint64_t GetDebugBytes();

    void AddDebugCurrentFPS(double fps);
    double GetDebugCurrentFPS();

    void AddDebugAverageFPS(double fps);
    double GetDebugAverageFPS();

    void AddDebugPublicQueue(uint32_t nSize);
    uint32_t GetDebugPublicQueue();

    void AddDebugPrivateQueue(uint32_t nSize);
    uint32_t GetDebugPrivateQueue();

    void AddInfoLine(uint64_t nLine);
    uint64_t GetInfoLine();

    void AddInfoBytes(uint64_t nBytes);
    uint64_t GetInfoBytes();

    void AddInfoCurrentFPS(double fps);
    double GetInfoCurrentFPS();

    void AddInfoAverageFPS(double fps);
    double GetInfoAverageFPS();

    void AddInfoPublicQueue(uint32_t nSize);
    uint32_t GetInfoPublicQueue();

    void AddInfoPrivateQueue(uint32_t nSize);
    uint32_t GetInfoPrivateQueue();

    void AddWarnLine(uint64_t nLine);
    uint64_t GetWarnLine();

    void AddWarnBytes(uint64_t nBytes);
    uint64_t GetWarnBytes();

    void AddWarnCurrentFPS(double fps);
    double GetWarnCurrentFPS();

    void AddWarnAverageFPS(double fps);
    double GetWarnAverageFPS();

    void AddWarnPublicQueue(uint32_t nSize);
    uint32_t GetWarnPublicQueue();

    void AddWarnPrivateQueue(uint32_t nSize);
    uint32_t GetWarnPrivateQueue();

    void AddErrorLine(uint64_t nLine);
    uint64_t GetErrorLine();

    void AddErrorBytes(uint64_t nBytes);
    uint64_t GetErrorBytes();

    void AddErrorCurrentFPS(double fps);
    double GetErrorCurrentFPS();

    void AddErrorAverageFPS(double fps);
    double GetErrorAverageFPS();

    void AddErrorPublicQueue(uint32_t nSize);
    uint32_t GetErrorPublicQueue();

    void AddErrorPrivateQueue(uint32_t nSize);
    uint32_t GetErrorPrivateQueue();

    void AddFatalLine(uint64_t nLine);
    uint64_t GetFatalLine();

    void AddFatalBytes(uint64_t nBytes);
    uint64_t GetFatalBytes();

    void AddFatalCurrentFPS(double fps);
    double GetFatalCurrentFPS();

    void AddFatalAverageFPS(double fps);
    double GetFatalAverageFPS();

    void AddFatalPublicQueue(uint32_t nSize);
    uint32_t GetFatalPublicQueue();

    void AddFatalPrivateQueue(uint32_t nSize);
    uint32_t GetFatalPrivateQueue();

    void AddMainLine(uint64_t nLine);
    uint64_t GetMainLine();

    void AddMainBytes(uint64_t nBytes);
    uint64_t GetMainBytes();

    void AddMainCurrentFPS(double fps);
    double GetMainCurrentFPS();

    void AddMainAverageFPS(double fps);
    double GetMainAverageFPS();

    void AddMainPublicDQueue(uint32_t nSize);
    void AddMainPublicTQueue(uint32_t nSize);
    void AddMainPublicIQueue(uint32_t nSize);
    void AddMainPublicWQueue(uint32_t nSize);
    void AddMainPublicEQueue(uint32_t nSize);
    void AddMainPublicFQueue(uint32_t nSize);
    void AddMainPrivateQueue(uint32_t nSize);

    uint32_t GetMainPublicDQueue();
    uint32_t GetMainPublicTQueue();
    uint32_t GetMainPublicIQueue();
    uint32_t GetMainPublicWQueue();
    uint32_t GetMainPublicEQueue();
    uint32_t GetMainPublicFQueue();
    uint32_t GetMainPrivateQueue();

    void AddRemoteLine(uint64_t nLine);
    uint64_t GetRemoteLine();

    void AddRemoteBytes(uint64_t nBytes);
    uint64_t GetRemoteBytes();

    void AddRemoteCurrentFPS(double fps);
    double GetRemoteCurrentFPS();

    void AddRemoteAverageFPS(double fps);
    double GetRemoteAverageFPS();

    void AddRemotePublicDQueue(uint32_t nSize);
    void AddRemotePublicTQueue(uint32_t nSize);
    void AddRemotePublicIQueue(uint32_t nSize);
    void AddRemotePublicWQueue(uint32_t nSize);
    void AddRemotePublicEQueue(uint32_t nSize);
    void AddRemotePublicFQueue(uint32_t nSize);
    void AddRemotePrivateQueue(uint32_t nSize);

    uint32_t GetRemotePublicDQueue();
    uint32_t GetRemotePublicTQueue();
    uint32_t GetRemotePublicIQueue();
    uint32_t GetRemotePublicWQueue();
    uint32_t GetRemotePublicEQueue();
    uint32_t GetRemotePublicFQueue();
    uint32_t GetRemotePrivateQueue();

    void AddSysLine(uint64_t nLine);
    uint64_t GetSysLine();

    void AddSysBytes(uint64_t nBytes);
    uint64_t GetSysBytes();

    void AddSysCurrentFPS(double fps);
    double GetSysCurrentFPS();

    void AddSysAverageFPS(double fps);
    double GetSysAverageFPS();

    void AddSysPublicQueue(uint32_t nSize);
    uint32_t GetSysPublicQueue();

    void AddSysPrivateQueue(uint32_t nSize);
    uint32_t GetSysPrivateQueue();

    void Reset();
    //////////////////////////////////////////////////////////////////////////
    //Singleton.
    static SharePtr<CYStatistics> GetInstance();
    static void FreeInstance();

private:
    std::atomic_uint64_t m_nTotalLine = 0;          // The total number of lines written to the log.
    std::atomic_uint64_t m_nTotalByte = 0;          // Total bytes written to the log.
    std::atomic<double>  m_fTotalCurrentFps = 0;    // Total fps written to the log.
    std::atomic<double>  m_fTotalAverageFps = 0;    // Total fps written to the log.
    std::atomic_uint32_t m_nTotalPublicQueue = 0;   // Total public queue length.
    std::atomic_uint32_t m_nTotalPrivateQueue = 0;  // Total private queue length.

    std::atomic_uint64_t m_nConsoleLine = 0;        // The total number of lines written to the console log.
    std::atomic_uint64_t m_nConsoleByte = 0;        // Total bytes written to the console log.
    std::atomic<double>  m_fConsoleCurrentFps = 0;  // Total fps written to the console log.
    std::atomic<double>  m_fConsoleAverageFps = 0;  // Total fps written to the console log.
    std::atomic_uint32_t m_nConsolePublicDQueue = 0;// The console public debug queue length.
    std::atomic_uint32_t m_nConsolePublicTQueue = 0;// The console public trace queue length.
    std::atomic_uint32_t m_nConsolePublicIQueue = 0;// The console public info queue length.
    std::atomic_uint32_t m_nConsolePublicWQueue = 0;// The console public warn queue length.
    std::atomic_uint32_t m_nConsolePublicEQueue = 0;// The console public error queue length.
    std::atomic_uint32_t m_nConsolePublicFQueue = 0;// The console public fatal queue length.
    std::atomic_uint32_t m_nConsolePrivateQueue = 0;// The console private queue length.

    std::atomic_uint64_t m_nTraceLine = 0;          // The total number of lines written to the trace log.
    std::atomic_uint64_t m_nTraceByte = 0;          // Total bytes written to the trace log.
    std::atomic<double>  m_fTraceCurrentFps = 0;    // Total fps written to the trace log.
    std::atomic<double>  m_fTraceAverageFps = 0;    // Total fps written to the trace log.
    std::atomic_uint32_t m_nTracePublicQueue = 0;   // The trace public queue length.
    std::atomic_uint32_t m_nTracePrivateQueue = 0;  // The trace private queue length.

    std::atomic_uint64_t m_nDebugLine = 0;          // The total number of lines written to the debug log.
    std::atomic_uint64_t m_nDebugByte = 0;          // Total bytes written to the debug log.
    std::atomic<double>  m_fDebugCurrentFps = 0;    // Total fps written to the debug log.
    std::atomic<double>  m_fDebugAverageFps = 0;    // Total fps written to the debug log.
    std::atomic_uint32_t m_nDebugPublicQueue = 0;   // The debug public queue length.
    std::atomic_uint32_t m_nDebugPrivateQueue = 0;  // The debug private queue length.

    std::atomic_uint64_t m_nInfoLine = 0;           // The total number of lines written to the info log.
    std::atomic_uint64_t m_nInfoByte = 0;           // Total bytes written to the info log.
    std::atomic<double>  m_fInfoCurrentFps = 0;     // Total fps written to the info log.
    std::atomic<double>  m_fInfoAverageFps = 0;     // Total fps written to the info log.
    std::atomic_uint32_t m_nInfoPublicQueue = 0;    // The info public queue length.
    std::atomic_uint32_t m_nInfoPrivateQueue = 0;   // The info private queue length.

    std::atomic_uint64_t m_nWarnLine = 0;           // The total number of lines written to the warn log.
    std::atomic_uint64_t m_nWarnByte = 0;           // Total bytes written to the warn log.
    std::atomic<double>  m_fWarnCurrentFps = 0;     // Total fps written to the warn log.
    std::atomic<double>  m_fWarnAverageFps = 0;     // Total fps written to the warn log.
    std::atomic_uint32_t m_nWarnPublicQueue = 0;    // The warn public queue length.
    std::atomic_uint32_t m_nWarnPrivateQueue = 0;   // The warn private queue length.

    std::atomic_uint64_t m_nErrorLine = 0;          // The total number of lines written to the error log.
    std::atomic_uint64_t m_nErrorByte = 0;          // Total bytes written to the error log.
    std::atomic<double>  m_fErrorCurrentFps = 0;    // Total fps written to the error log.
    std::atomic<double>  m_fErrorAverageFps = 0;    // Total fps written to the error log.
    std::atomic_uint32_t m_nErrorPublicQueue = 0;   // The error public queue length.
    std::atomic_uint32_t m_nErrorPrivateQueue = 0;  // The error private queue length.

    std::atomic_uint64_t m_nFatalLine = 0;          // The total number of lines written to the fatal log.
    std::atomic_uint64_t m_nFatalByte = 0;          // Total bytes written to the fatal log.
    std::atomic<double>  m_fFatalCurrentFps = 0;    // Total fps written to the fatal log.
    std::atomic<double>  m_fFatalAverageFps = 0;    // Total fps written to the fatal log.
    std::atomic_uint32_t m_nFatalPublicQueue = 0;   // The fatal public queue length.
    std::atomic_uint32_t m_nFatalPrivateQueue = 0;  // The fatal private queue length.

    std::atomic_uint64_t m_nMainLine = 0;           // The total number of lines written to the main log.
    std::atomic_uint64_t m_nMainByte = 0;           // Total bytes written to the main log.
    std::atomic<double>  m_fMainCurrentFps = 0;     // Total fps written to the main log.
    std::atomic<double>  m_fMainAverageFps = 0;     // Total fps written to the main log.
    std::atomic_uint32_t m_nMainPublicDQueue = 0;   // The main public debug queue length.
    std::atomic_uint32_t m_nMainPublicTQueue = 0;   // The main public trace queue length.
    std::atomic_uint32_t m_nMainPublicIQueue = 0;   // The main public info queue length.
    std::atomic_uint32_t m_nMainPublicWQueue = 0;   // The main public warn queue length.
    std::atomic_uint32_t m_nMainPublicEQueue = 0;   // The main public error queue length.
    std::atomic_uint32_t m_nMainPublicFQueue = 0;   // The main public queue length.
    std::atomic_uint32_t m_nMainPrivateQueue = 0;   // The main private queue length.

    std::atomic_uint64_t m_nRemoteLine = 0;         // The total number of lines written to the remote log.
    std::atomic_uint64_t m_nRemoteByte = 0;         // Total bytes written to the remote log.
    std::atomic<double>  m_fRemoteCurrentFps = 0;   // Total fps written to the remote log.
    std::atomic<double>  m_fRemoteAverageFps = 0;   // Total fps written to the remote log.
    std::atomic_uint32_t m_nRemotePublicDQueue = 0; // The remote public debug queue length.
    std::atomic_uint32_t m_nRemotePublicTQueue = 0; // The remote public trace queue length.
    std::atomic_uint32_t m_nRemotePublicIQueue = 0; // The remote public info queue length.
    std::atomic_uint32_t m_nRemotePublicWQueue = 0; // The remote public warn queue length.
    std::atomic_uint32_t m_nRemotePublicEQueue = 0; // The remote public error queue length.
    std::atomic_uint32_t m_nRemotePublicFQueue = 0; // The remote public  queue length.
    std::atomic_uint32_t m_nRemotePrivateQueue = 0; // The remote private queue length.

    std::atomic_uint64_t m_nSysLine = 0;            // The total number of lines written to the system event log.
    std::atomic_uint64_t m_nSysByte = 0;            // Total bytes written to the system event log.
    std::atomic<double>  m_fSysCurrentFps = 0;      // Total fps written to the system log.
    std::atomic<double>  m_fSysAverageFps = 0;      // Total fps written to the system log.
    std::atomic_uint32_t m_nSysPublicQueue = 0;     // The system public queue length.
    std::atomic_uint32_t m_nSysPrivateQueue = 0;    // The system private queue length.

    static SharePtr<CYStatistics> m_ptrInstance;
};

#define Statistics() CYStatistics::GetInstance()
#define Statistics_Free() CYStatistics::FreeInstance()

CYLOGGER_NAMESPACE_END

#endif //__CY_STATISTICS_HPP__