#include "Src/Statistics/CYStatistics.hpp"

CYLOGGER_NAMESPACE_BEGIN

SharePtr<CYStatistics> CYStatistics::m_ptrInstance;
//
// memory_order_relaxed	不对执行顺序做任何保证
// memory_order_acquire	本线程中，所有后续的读操作必须在本条原子操作完成后执行
// memory_order_release	本线程中，所有之前的写操作完成后才能执行本条原子操作
// memory_order_acq_rel	同时包含memory_order_acquire和memory_order_release标记
// memory_order_consume	本线程中，所有后续的有关本原子类型的操作，必须在本条原子操作完成后执行
// memory_order_seq_cst	全部存取都按顺序执行
// memory_order_acquire保证配对使用的 memory_order_release操作之前的所有原子和非原子的写入操作有效，但memory_order_consume仅保证配对使用的 memory_order_release操作之前的所有原子的写入操作和依赖原子写入的非原子操作有效。
//

void CYStatistics::AddTotalLine(uint64_t nLine)
{
    m_nTotalLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetTotalLine()
{
    return m_nTotalLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddTotalBytes(uint64_t nBytes)
{
    m_nTotalByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetTotalBytes()
{
    return m_nTotalByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddTotalCurrentFPS(double fps)
{
    m_fTotalCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetTotalCurrentFPS()
{
    return m_fTotalCurrentFps.load(std::memory_order_relaxed);
}


void CYStatistics::AddTotalAverageFPS(double fps)
{
    m_fTotalAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetTotalAverageFPS()
{
    return m_fTotalAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddTotalPublicQueue(uint32_t nSize)
{
    m_nTotalPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetTotalPublicQueue()
{
    return m_nTotalPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddTotalPrivateQueue(uint32_t nSize)
{
    m_nTotalPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetTotalPrivateQueue()
{
    return m_nTotalPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddConsoleLine(uint64_t nLine)
{
    m_nConsoleLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetConsoleLine()
{
    return m_nConsoleLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddConsoleBytes(uint64_t nBytes)
{
    m_nConsoleByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetConsoleBytes()
{
    return m_nConsoleByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddConsoleCurrentFPS(double fps)
{
    m_fConsoleCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetConsoleCurrentFPS()
{
    return m_fConsoleCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddConsoleAverageFPS(double fps)
{
    m_fConsoleAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetConsoleAverageFPS()
{
    return m_fConsoleAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicDQueue(uint32_t nSize)
{
     m_nConsolePublicDQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicTQueue(uint32_t nSize)
{
    m_nConsolePublicTQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicIQueue(uint32_t nSize)
{
    m_nConsolePublicIQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicWQueue(uint32_t nSize)
{
    m_nConsolePublicWQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicEQueue(uint32_t nSize)
{
    m_nConsolePublicEQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePublicFQueue(uint32_t nSize)
{
    m_nConsolePublicFQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddConsolePrivateQueue(uint32_t nSize)
{
    m_nConsolePrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicDQueue()
{
    return m_nConsolePublicDQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicTQueue()
{
    return m_nConsolePublicTQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicIQueue()
{
    return m_nConsolePublicIQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicWQueue()
{
    return m_nConsolePublicWQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicEQueue()
{
    return m_nConsolePublicEQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePublicFQueue()
{
    return m_nConsolePublicFQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetConsolePrivateQueue()
{
    return m_nConsolePrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddTraceLine(uint64_t nLine)
{
    m_nTraceLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetTraceLine()
{
    return m_nTraceLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddTraceBytes(uint64_t nBytes)
{
    m_nTraceByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetTraceBytes()
{
    return m_nTraceByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddTraceCurrentFPS(double fps)
{
    m_fTraceCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetTraceCurrentFPS()
{
    return m_fTraceCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddTraceAverageFPS(double fps)
{
    m_fTraceAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetTraceAverageFPS()
{
    return m_fTraceAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddTracePublicQueue(uint32_t nSize)
{
    m_nTracePublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetTracePublicQueue()
{
    return m_nTracePublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddTracePrivateQueue(uint32_t nSize)
{
    m_nTracePrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetTracePrivateQueue()
{
    return m_nTracePrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugLine(uint64_t nLine)
{
    m_nDebugLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetDebugLine()
{
    return m_nDebugLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugBytes(uint64_t nBytes)
{
    m_nDebugByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetDebugBytes()
{
    return m_nDebugByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugCurrentFPS(double fps)
{
    m_fDebugCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetDebugCurrentFPS()
{
    return m_fDebugCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugAverageFPS(double fps)
{
    m_fDebugAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetDebugAverageFPS()
{
    return m_fDebugAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugPublicQueue(uint32_t nSize)
{
    m_nDebugPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetDebugPublicQueue()
{
    return m_nDebugPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddDebugPrivateQueue(uint32_t nSize)
{
    m_nDebugPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetDebugPrivateQueue()
{
    return m_nDebugPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoLine(uint64_t nLine)
{
    m_nInfoLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetInfoLine()
{
    return m_nInfoLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoBytes(uint64_t nBytes)
{
    m_nInfoByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetInfoBytes()
{
    return m_nInfoByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoCurrentFPS(double fps)
{
    m_fInfoCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetInfoCurrentFPS()
{
    return m_fInfoCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoAverageFPS(double fps)
{
    m_fInfoAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetInfoAverageFPS()
{
    return m_fInfoAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoPublicQueue(uint32_t nSize)
{
    m_nInfoPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetInfoPublicQueue()
{
    return m_nInfoPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddInfoPrivateQueue(uint32_t nSize)
{
    m_nInfoPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetInfoPrivateQueue()
{
    return m_nInfoPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnLine(uint64_t nLine)
{
    m_nWarnLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetWarnLine()
{
    return m_nWarnLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnBytes(uint64_t nBytes)
{
    m_nWarnByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetWarnBytes()
{
    return m_nWarnByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnCurrentFPS(double fps)
{
    m_fWarnCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetWarnCurrentFPS()
{
    return m_fWarnCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnAverageFPS(double fps)
{
    m_fWarnAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetWarnAverageFPS()
{
    return m_fWarnAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnPublicQueue(uint32_t nSize)
{
    m_nWarnPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetWarnPublicQueue()
{
    return m_nWarnPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddWarnPrivateQueue(uint32_t nSize)
{
    m_nWarnPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetWarnPrivateQueue()
{
    return m_nWarnPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorLine(uint64_t nLine)
{
    m_nErrorLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetErrorLine()
{
    return m_nErrorLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorBytes(uint64_t nBytes)
{
    m_nErrorByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetErrorBytes()
{
    return m_nErrorByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorCurrentFPS(double fps)
{
    m_fErrorCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetErrorCurrentFPS()
{
    return m_fErrorCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorAverageFPS(double fps)
{
    m_fErrorAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetErrorAverageFPS()
{
    return m_fErrorAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorPublicQueue(uint32_t nSize)
{
    m_nErrorPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetErrorPublicQueue()
{
    return m_nErrorPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddErrorPrivateQueue(uint32_t nSize)
{
    m_nErrorPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetErrorPrivateQueue()
{
    return m_nErrorPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalLine(uint64_t nLine)
{
    m_nFatalLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetFatalLine()
{
    return m_nFatalLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalBytes(uint64_t nBytes)
{
    m_nFatalByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetFatalBytes()
{
    return m_nFatalByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalCurrentFPS(double fps)
{
    m_fFatalCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetFatalCurrentFPS()
{
    return m_fFatalCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalAverageFPS(double fps)
{
    m_fFatalAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetFatalAverageFPS()
{
    return m_fFatalAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalPublicQueue(uint32_t nSize)
{
    m_nFatalPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetFatalPublicQueue()
{
    return m_nFatalPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddFatalPrivateQueue(uint32_t nSize)
{
    m_nFatalPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetFatalPrivateQueue()
{
    return m_nFatalPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddMainLine(uint64_t nLine)
{
    m_nMainLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetMainLine()
{
    return m_nMainLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddMainBytes(uint64_t nBytes)
{
    m_nMainByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetMainBytes()
{
    return m_nMainByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddMainCurrentFPS(double fps)
{
    m_fMainCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetMainCurrentFPS()
{
    return m_fMainCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddMainAverageFPS(double fps)
{
    m_fMainAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetMainAverageFPS()
{
    return m_fMainAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicDQueue(uint32_t nSize)
{
    m_nMainPublicDQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicTQueue(uint32_t nSize)
{
    m_nMainPublicTQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicIQueue(uint32_t nSize)
{
    m_nMainPublicIQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicWQueue(uint32_t nSize)
{
    m_nMainPublicWQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicEQueue(uint32_t nSize)
{
    m_nMainPublicEQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPublicFQueue(uint32_t nSize)
{
    m_nMainPublicFQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddMainPrivateQueue(uint32_t nSize)
{
    m_nMainPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicDQueue()
{
    return m_nMainPublicDQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicTQueue()
{
    return m_nMainPublicTQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicIQueue()
{
    return m_nMainPublicIQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicWQueue()
{
    return m_nMainPublicWQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicEQueue()
{
    return m_nMainPublicEQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPublicFQueue()
{
    return m_nMainPublicFQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetMainPrivateQueue()
{
    return m_nMainPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddRemoteLine(uint64_t nLine)
{
    m_nRemoteLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetRemoteLine()
{
    return m_nRemoteLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddRemoteBytes(uint64_t nBytes)
{
    m_nRemoteByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetRemoteBytes()
{
    return m_nRemoteByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddRemoteCurrentFPS(double fps)
{
    m_fRemoteCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetRemoteCurrentFPS()
{
    return m_fRemoteCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddRemoteAverageFPS(double fps)
{
    m_fRemoteAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetRemoteAverageFPS()
{
    return m_fRemoteAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicDQueue(uint32_t nSize)
{
    m_nRemotePublicDQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicTQueue(uint32_t nSize)
{
    m_nRemotePublicTQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicIQueue(uint32_t nSize)
{
    m_nRemotePublicIQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicWQueue(uint32_t nSize)
{
    m_nRemotePublicWQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicEQueue(uint32_t nSize)
{
    m_nRemotePublicEQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePublicFQueue(uint32_t nSize)
{
    m_nRemotePublicFQueue.exchange(nSize, std::memory_order_relaxed);
}

void CYStatistics::AddRemotePrivateQueue(uint32_t nSize)
{
    m_nRemotePrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicDQueue()
{
    return m_nRemotePublicDQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicTQueue()
{
    return m_nRemotePublicTQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicIQueue()
{
    return m_nRemotePublicIQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicWQueue()
{
    return m_nRemotePublicWQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicEQueue()
{
    return m_nRemotePublicEQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePublicFQueue()
{
    return m_nRemotePublicFQueue.load(std::memory_order_relaxed);
}

uint32_t CYStatistics::GetRemotePrivateQueue()
{
    return m_nRemotePrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysLine(uint64_t nLine)
{
    m_nSysLine.fetch_add(nLine, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetSysLine()
{
    return m_nSysLine.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysBytes(uint64_t nBytes)
{
    m_nSysByte.fetch_add(nBytes, std::memory_order_relaxed);
}

uint64_t CYStatistics::GetSysBytes()
{
    return m_nSysByte.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysCurrentFPS(double fps)
{
    m_fSysCurrentFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetSysCurrentFPS()
{
    return m_fSysCurrentFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysAverageFPS(double fps)
{
    m_fSysAverageFps.exchange(fps, std::memory_order_relaxed);
}

double CYStatistics::GetSysAverageFPS()
{
    return m_fSysAverageFps.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysPublicQueue(uint32_t nSize)
{
    m_nSysPublicQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetSysPublicQueue()
{
    return m_nSysPublicQueue.load(std::memory_order_relaxed);
}

void CYStatistics::AddSysPrivateQueue(uint32_t nSize)
{
    m_nSysPrivateQueue.exchange(nSize, std::memory_order_relaxed);
}

uint32_t CYStatistics::GetSysPrivateQueue()
{
    return m_nSysPrivateQueue.load(std::memory_order_relaxed);
}

void CYStatistics::Reset()
{
    m_nTotalLine.exchange(0, std::memory_order_relaxed);
    m_nTotalByte.exchange(0, std::memory_order_relaxed);
    m_fTotalCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fTotalAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nConsoleLine.exchange(0, std::memory_order_relaxed);
    m_nConsoleByte.exchange(0, std::memory_order_relaxed);
    m_fConsoleCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fConsoleAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nTraceLine.exchange(0, std::memory_order_relaxed);
    m_nTraceByte.exchange(0, std::memory_order_relaxed);
    m_fTraceCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fTraceAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nDebugLine.exchange(0, std::memory_order_relaxed);
    m_nDebugByte.exchange(0, std::memory_order_relaxed);
    m_fDebugCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fDebugAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nInfoLine.exchange(0, std::memory_order_relaxed);
    m_nInfoByte.exchange(0, std::memory_order_relaxed);
    m_fInfoCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fInfoAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nWarnLine.exchange(0, std::memory_order_relaxed);
    m_nWarnByte.exchange(0, std::memory_order_relaxed);
    m_fWarnCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fWarnAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nErrorLine.exchange(0, std::memory_order_relaxed);
    m_nErrorByte.exchange(0, std::memory_order_relaxed);
    m_fErrorCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fErrorAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nFatalLine.exchange(0, std::memory_order_relaxed);
    m_nFatalByte.exchange(0, std::memory_order_relaxed);
    m_fFatalCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fFatalAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nMainLine.exchange(0, std::memory_order_relaxed);
    m_nMainByte.exchange(0, std::memory_order_relaxed);
    m_fMainCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fMainAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nRemoteLine.exchange(0, std::memory_order_relaxed);
    m_nRemoteByte.exchange(0, std::memory_order_relaxed);
    m_fRemoteCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fRemoteAverageFps.exchange(0.0, std::memory_order_relaxed);

    m_nSysLine.exchange(0, std::memory_order_relaxed);
    m_nSysByte.exchange(0, std::memory_order_relaxed);
    m_fSysCurrentFps.exchange(0.0, std::memory_order_relaxed);
    m_fSysAverageFps.exchange(0.0, std::memory_order_relaxed);
}

//////////////////////////////////////////////////////////////////////////
SharePtr<CYStatistics> CYStatistics::GetInstance()
{
    if (!m_ptrInstance)
    {
        m_ptrInstance = MakeShared<CYStatistics>();
    }
    return m_ptrInstance;
}

void CYStatistics::FreeInstance()
{
    m_ptrInstance.reset();
}

CYLOGGER_NAMESPACE_END