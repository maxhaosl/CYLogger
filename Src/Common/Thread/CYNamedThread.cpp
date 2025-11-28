#include "Common/Thread/CYNamedThread.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYNamedThread::CYNamedThread(std::string_view strName) noexcept
    : m_strName(strName)
{
}

CYNamedThread::~CYNamedThread() noexcept
{
}

bool CYNamedThread::StartThread()
{
    if (this->m_thread.joinable())
        return false;

#ifdef _WIN32
    std::stop_source objStopSource;
    m_objStopSource.swap(objStopSource);
    m_objToken = m_objStopSource.get_token();
#else
    m_bIsRunning = true;
#endif
    this->m_thread = cy_jthread([this]() { Entry(); });
    return this->m_thread.joinable();
}

bool CYNamedThread::StopThread()
{
#ifdef _WIN32
    m_objStopSource.request_stop();
#else
    m_bIsRunning = false;
#endif
    if (this->m_thread.joinable())
    {
        this->m_thread.join();
        return true;
    }
    return false;
}

cy_jthread::id CYNamedThread::GetId() const noexcept
{
    return this->m_thread.get_id();
}

bool CYNamedThread::IsRunning() const noexcept
{
#ifdef _WIN32
    return !m_objToken.stop_requested();
#else
    return m_bIsRunning;
#endif
}

void CYNamedThread::Wait()
{
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

void CYNamedThread::Entry()
{
    SetName(m_strName);
    Run();
}

void CYNamedThread::SetThreadName(std::string_view strName) noexcept
{
    m_strName = strName;
}

#ifdef CYLOGGER_WIN_OS

#include <Windows.h>
void CYNamedThread::SetName(std::string_view strName) noexcept
{
    const std::wstring utf16_name(strName.begin(), strName.end());
    SetThreadDescription(GetCurrentThread(), utf16_name.data());
}

#elif defined(CYLOGGER_MINGW_OS)

#include <pthread.h>
void CYNamedThread::SetName(std::string_view strName) noexcept
{
    ::pthread_setname_np(::pthread_self(), strName.data());
}

#elif defined(CYLOGGER_UNIX_OS)

#include <pthread.h>
void CYNamedThread::SetName(std::string_view strName) noexcept
{
    ::pthread_setname_np(::pthread_self(), strName.data());
}

#elif defined(CYLOGGER_MAC_OS)

#include <pthread.h>
void CYNamedThread::SetName(std::string_view strName) noexcept
{
    ::pthread_setname_np(strName.data());
}

#endif

CYLOGGER_NAMESPACE_END