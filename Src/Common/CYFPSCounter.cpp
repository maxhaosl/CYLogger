#include "Src/Common/CYFPSCounter.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYFPSCounter::CYFPSCounter(int nMeasureDuration)
    : m_nMeasureDuration(nMeasureDuration)
{
}

void CYFPSCounter::StartCounter()
{
    m_nFrameCount[0] = 0;
    m_nFrameCount[1] = 0;
    m_fCurrentFps = 0.0;
    m_fAverageFps = 0.0;
    m_tpStartTime1 = {};
    m_tpStartTime0 = std::chrono::high_resolution_clock::now();
}

void CYFPSCounter::UpdateCounter()
{
    auto tpFrameStart = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> tpElapsedSeconds0 = tpFrameStart - m_tpStartTime0;

    if (tpElapsedSeconds0.count() >= 1.0)
    {
        m_fCurrentFps = m_nFrameCount[0] / tpElapsedSeconds0.count();
        UpdateAverageFPS(0);

        // Reset counter and start time
        m_nFrameCount[0] = 0;
        m_tpStartTime0 = tpFrameStart;
    }

    std::chrono::duration<double> tpElapsedSeconds1 = tpFrameStart - m_tpStartTime1;

    if (tpElapsedSeconds1.count() >= 1.0)
    {
        m_fCurrentFps = m_nFrameCount[1] / tpElapsedSeconds1.count();
        UpdateAverageFPS(1);

        // Reset counter and start time
        m_nFrameCount[1] = 0;
        m_tpStartTime1 = tpFrameStart;
    }

    // Check if the specified measurement time is reached, if so, output the average FPS and exit the loop.
    if (tpElapsedSeconds0.count() >= m_nMeasureDuration)
    {
        m_fAverageFps = m_nFrameCount[0] / tpElapsedSeconds0.count();
        m_tpStartTime0 = std::chrono::high_resolution_clock::now();
        m_nFrameCount[0] = 0;
    }

    if (tpElapsedSeconds0.count() >= m_nMeasureDuration / 2)
    {
        m_tpStartTime1 = std::chrono::high_resolution_clock::now();
    }

    if (tpElapsedSeconds1.count() >= m_nMeasureDuration)
    {
        m_fAverageFps = m_nFrameCount[1] / tpElapsedSeconds1.count();
        m_tpStartTime1 = std::chrono::high_resolution_clock::now();
        m_nFrameCount[1] = 0;
    }

    m_nFrameCount[0]++;
    m_nFrameCount[1]++;
}

void CYFPSCounter::UpdateAverageFPS(int nIndex)
{
    m_fAverageFps = (m_fAverageFps * (m_nFrameCount[nIndex] - 1) + m_fCurrentFps) / m_nFrameCount[nIndex];
}

double CYFPSCounter::GetCurrentFPS() const
{
    return m_fCurrentFps;
}

double CYFPSCounter::GetAverageFPS() const
{
    return m_fAverageFps;
}


CYLOGGER_NAMESPACE_END