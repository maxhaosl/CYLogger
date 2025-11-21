#include "CYCoroutine/CYCoroutine.hpp"
#include "Entity/Appender/CYLoggerBufferAppender.hpp"
#include "Statistics/CYStatistics.hpp"
#include "Common/Exception/CYExceptionLogFile.hpp"

CYLOGGER_NAMESPACE_BEGIN

/**
	* @brief Constructor.
*/
CYLoggerBufferAppender::CYLoggerBufferAppender(std::string_view strName) noexcept
	: CYLoggerBaseAppender(strName)
{

}

/**
	* @brief Destructor.
*/
CYLoggerBufferAppender::~CYLoggerBufferAppender() noexcept
{

}

/**
* @brief Put message to the Log
*/
void CYLoggerBufferAppender::WriteLog(const SharePtr<CYBaseMessage>& ptrMessage)
{
    EXCEPTION_BEGIN
    {
        switch (ptrMessage->GetMsgType())
        {
        case LOG_TYPE_DEBUG:
        {
            LockGuard locker(m_mutexPublicDebugMessage);
            m_lstPublicDebugMessage.push_back(ptrMessage);
            break;
        }
        case LOG_TYPE_TRACE:
        {
            LockGuard locker(m_mutexPublicTraceMessage);
            m_lstPublicTraceMessage.push_back(ptrMessage);
            break;
        }
        case LOG_TYPE_INFO:
        {
            LockGuard locker(m_mutexPublicInfoMessage);
            m_lstPublicInfoMessage.push_back(ptrMessage);
            break;
        }
        case LOG_TYPE_WARN:
        {
            LockGuard locker(m_mutexPublicWarnMessage);
            m_lstPublicWarnMessage.push_back(ptrMessage);
            break;
        }
        case LOG_TYPE_ERROR:
        {
            LockGuard locker(m_mutexPublicErrMessage);
            m_lstPublicErrMessage.push_back(ptrMessage);
            break;
        }
        case LOG_TYPE_FATAL:
        {
            LockGuard locker(m_mutexPublicFatalMessage);
            m_lstPublicFatalMessage.push_back(ptrMessage);
            break;
        }
        default:
            assert(0);
            break;
        }
    }
    EXCEPTION_END

    UpdatePublicStats();
}

/**
 * @brief Update Public Statistics.
*/
void CYLoggerBufferAppender::UpdatePublicStats()
{
    ELogType eLogType = this->GetId();
    switch (eLogType)
    {
    case LOG_TYPE_NONE:
        Statistics()->AddConsolePublicDQueue(static_cast<uint32_t>(m_lstPublicDebugMessage.size()));
        Statistics()->AddConsolePublicTQueue(static_cast<uint32_t>(m_lstPublicTraceMessage.size()));
        Statistics()->AddConsolePublicIQueue(static_cast<uint32_t>(m_lstPublicInfoMessage.size()));
        Statistics()->AddConsolePublicWQueue(static_cast<uint32_t>(m_lstPublicWarnMessage.size()));
        Statistics()->AddConsolePublicEQueue(static_cast<uint32_t>(m_lstPublicErrMessage.size()));
        Statistics()->AddConsolePublicFQueue(static_cast<uint32_t>(m_lstPublicFatalMessage.size()));
        break;
    case LOG_TYPE_TRACE:
        assert(0);
        break;
    case LOG_TYPE_DEBUG:
        assert(0);
        break;
    case LOG_TYPE_INFO:
        assert(0);
        break;
    case LOG_TYPE_WARN:
        assert(0);
        break;
    case LOG_TYPE_ERROR:
        assert(0);
        break;
    case LOG_TYPE_FATAL:
        assert(0);
        break;
    case LOG_TYPE_MAIN:
        Statistics()->AddMainPublicDQueue(static_cast<uint32_t>(m_lstPublicDebugMessage.size()));
        Statistics()->AddMainPublicTQueue(static_cast<uint32_t>(m_lstPublicTraceMessage.size()));
        Statistics()->AddMainPublicIQueue(static_cast<uint32_t>(m_lstPublicInfoMessage.size()));
        Statistics()->AddMainPublicWQueue(static_cast<uint32_t>(m_lstPublicWarnMessage.size()));
        Statistics()->AddMainPublicEQueue(static_cast<uint32_t>(m_lstPublicErrMessage.size()));
        Statistics()->AddMainPublicFQueue(static_cast<uint32_t>(m_lstPublicFatalMessage.size()));
        break;
    case LOG_TYPE_REMOTE:
        Statistics()->AddRemotePublicDQueue(static_cast<uint32_t>(m_lstPublicDebugMessage.size()));
        Statistics()->AddRemotePublicTQueue(static_cast<uint32_t>(m_lstPublicTraceMessage.size()));
        Statistics()->AddRemotePublicIQueue(static_cast<uint32_t>(m_lstPublicInfoMessage.size()));
        Statistics()->AddRemotePublicWQueue(static_cast<uint32_t>(m_lstPublicWarnMessage.size()));
        Statistics()->AddRemotePublicEQueue(static_cast<uint32_t>(m_lstPublicErrMessage.size()));
        Statistics()->AddRemotePublicFQueue(static_cast<uint32_t>(m_lstPublicFatalMessage.size()));
        break;
    case LOG_TYPE_SYS:
        assert(0);
        break;
    case LOG_TYPE_MAX:
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
}

/**
 * @brief Update Private Statistics.
*/
void CYLoggerBufferAppender::UpdatePrivateStats()
{
    ELogType eLogType = this->GetId();
    switch (eLogType)
    {
    case LOG_TYPE_NONE:
        Statistics()->AddConsolePrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_TRACE:
        assert(0);
        break;
    case LOG_TYPE_DEBUG:
        assert(0);
        break;
    case LOG_TYPE_INFO:
        assert(0);
        break;
    case LOG_TYPE_WARN:
        assert(0);
        break;
    case LOG_TYPE_ERROR:
        assert(0);
        break;
    case LOG_TYPE_FATAL:
        assert(0);
        break;
    case LOG_TYPE_MAIN:
        Statistics()->AddMainPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_REMOTE:
        Statistics()->AddRemotePrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_SYS:
        assert(0);
        break;
    case LOG_TYPE_MAX:
        assert(0);
        break;
    default:
        assert(0);
        break;
    }
}

/**
* @brief Coroutine work function.
*/
CYCOROUTINE_NAMESPACE::CYResult<int> CYLoggerBufferAppender::DoFlipBuffer(std::function<int()>&& fun)
{
    auto ret = co_await CYBackgroundCoro()->Submit([fun = std::forward<decltype(fun)>(fun)]() {
        try
        {
            return fun();
        }
        catch (...)
        {

        }
        return 0;
        });
    co_return ret;
}

/**
* @brief Flip buffering.
*/
void CYLoggerBufferAppender::FlipBuffer()
{
    auto objResult = DoFlipBuffer([&] {
        // Console Buffer.
        {
            LockGuard locker(m_mutexPublicDebugMessage);
            std::swap(m_lstPublicDebugMessage, m_lstPrivateDebugMessage);
        }

        // Trace Buffer.
        {
            LockGuard locker(m_mutexPublicTraceMessage);
            std::swap(m_lstPublicTraceMessage, m_lstPrivateTraceMessage);
        }

        // Info Buffer.
        {
            LockGuard locker(m_mutexPublicInfoMessage);
            std::swap(m_lstPublicInfoMessage, m_lstPrivateInfoMessage);
        }

        // Warn Buffer.
        {
            LockGuard locker(m_mutexPublicWarnMessage);
            std::swap(m_lstPublicWarnMessage, m_lstPrivateWarnMessage);
        }

        // Err Buffer.
        {
            LockGuard locker(m_mutexPublicErrMessage);
            std::swap(m_lstPublicErrMessage, m_lstPrivateErrMessage);
        }

        // Fatal Buffer.
        {
            LockGuard locker(m_mutexPublicFatalMessage);
            std::swap(m_lstPublicFatalMessage, m_lstPrivateFatalMessage);
        }

        while (!m_lstPrivateDebugMessage.empty())
        {
            auto ptrMessage = m_lstPrivateDebugMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateDebugMessage.pop_front();
        }

        while (!m_lstPrivateTraceMessage.empty())
        {
            auto ptrMessage = m_lstPrivateTraceMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateTraceMessage.pop_front();
        }

        while (!m_lstPrivateInfoMessage.empty())
        {
            auto ptrMessage = m_lstPrivateInfoMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateInfoMessage.pop_front();
        }

        while (!m_lstPrivateWarnMessage.empty())
        {
            auto ptrMessage = m_lstPrivateWarnMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateWarnMessage.pop_front();
        }

        while (!m_lstPrivateErrMessage.empty())
        {
            auto ptrMessage = m_lstPrivateErrMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateErrMessage.pop_front();
        }

        while (!m_lstPrivateFatalMessage.empty())
        {
            auto ptrMessage = m_lstPrivateFatalMessage.front();
            m_mapPrivateMessage.emplace(ptrMessage->GetTimeStamp().GetTime(), ptrMessage);
            m_lstPrivateFatalMessage.pop_front();
        }

        for (auto iterElem : m_mapPrivateMessage)
        {
            this->m_lstPrivMessage.push_back(iterElem.second);
        }

        UpdatePrivateStats();

        m_mapPrivateMessage.clear();

        return 0;
        });
    auto nRet = objResult.Get();
}

CYLOGGER_NAMESPACE_END