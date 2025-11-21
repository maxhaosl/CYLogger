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

#ifdef __cpp_lib_jthread
    std::stop_source objStopSource;
    m_objStopSource.swap(objStopSource);
    m_objToken = m_objStopSource.get_token();
    this->m_thread = std::jthread([this]() { Entry(); });
#else
    this->m_thread = std::thread(&CYNamedThread::Entry, this);
#endif
    return this->m_thread.joinable();
}

bool CYNamedThread::StopThread()
{
#ifdef __cpp_lib_jthread
    m_objStopSource.request_stop();
#endif
    if (this->m_thread.joinable())
    {
        this->m_thread.join();
        return true;
    }
    return false;
}

#ifdef __cpp_lib_jthread
std::jthread::id CYNamedThread::GetId() const noexcept
{
    return this->m_thread.get_id();
}
#else
std::thread::id CYNamedThread::GetId() const noexcept
{
    return this->m_thread.get_id();
}
#endif

bool CYNamedThread::IsRunning() const noexcept
{
#ifdef __cpp_lib_jthread
    return !m_objToken.stop_requested();
#else
    return this->m_thread.joinable();
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