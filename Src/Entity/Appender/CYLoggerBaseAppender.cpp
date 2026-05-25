#include "Entity/Appender/CYLoggerBaseAppender.hpp"
#include "Statistics/CYStatistics.hpp"
#include "Entity/Appender/CYLoggerAppenderDefine.hpp"
#include "Statistics/CYStatistics.hpp"
#include "Common/Exception/CYExceptionLogFile.hpp"
#if CYLOGGER_USE_CYCOROUTINE
#include "CYCoroutine/CYCoroutine.hpp"
#endif

#include <atlbase.h>
#include <atltrace.h>

CYLOGGER_NAMESPACE_BEGIN

CYLoggerBaseAppender::CYLoggerBaseAppender(std::string_view strName) noexcept
    : CYNamedThread(strName)
    , CYFPSCounter(LOG_FPS_CHECK_DURATION)
{
    CYFPSCounter::StartCounter();
}

CYLoggerBaseAppender::~CYLoggerBaseAppender() noexcept
{}

/**
 * @brief Start Log Thread.
*/
void CYLoggerBaseAppender::StartLogThread()
{
    CYNamedThread::StartThread();
}

/**
 * @brief Stop Log Thread.
*/
void CYLoggerBaseAppender::StopLogThread()
{
    m_objCondition.SignalOne();
    CYNamedThread::StopThread();
}

#if CYLOGGER_USE_CYCOROUTINE
/**
 * @brief Coroutine work funciton.
 */
static CYCOROUTINE_NAMESPACE::CYResult<std::tuple<int, int>> CoDoWork(std::function<std::tuple<int, int>()>&& fun)
{
    auto ret = co_await CYBackgroundCoro()->Submit([fun = std::forward<decltype(fun)>(fun)]() {
        try
        {
            return fun();
        }
        catch (...)
        {
        }
        return std::tuple<int, int>(0, 0);
        });

    co_return ret;
}
#endif

/**
* @brief Coroutine work function.
*/
std::tuple<int, int> CYLoggerBaseAppender::DoWork(std::function<std::tuple<int, int>()>&& fun)
{
#if CYLOGGER_USE_CYCOROUTINE
    return CoDoWork(std::forward<decltype(fun)>(fun)).Get();
#else
    try
    {
        return fun();
    }
    catch (...)
    {
    }
    return std::tuple<int, int>(0, 0);
#endif
}

/**
 * @brief Flip buffering.
*/
void CYLoggerBaseAppender::FlipBuffer()
{
    LockGuard locker(m_mutexPublicMessage);
    std::swap(m_lstPublicMessage, m_lstPrivMessage);
}

/**
 * @brief Update Public Statistics.
*/
void CYLoggerBaseAppender::UpdatePublicStats()
{
    ELogType eLogType = this->GetId();
    switch (eLogType)
    {
    case LOG_TYPE_NONE:
        assert(0);
        break;
    case LOG_TYPE_TRACE:
        Statistics()->AddTracePublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_DEBUG:
        Statistics()->AddDebugPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_INFO:
        Statistics()->AddInfoPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_WARN:
        Statistics()->AddWarnPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_ERROR:
        Statistics()->AddErrorPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_FATAL:
        Statistics()->AddFatalPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
        break;
    case LOG_TYPE_MAIN:
        assert(0);
        break;
    case LOG_TYPE_REMOTE:
        assert(0);
        break;
    case LOG_TYPE_SYS:
        Statistics()->AddSysPublicQueue(static_cast<uint32_t>(m_lstPublicMessage.size()));
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
void CYLoggerBaseAppender::UpdatePrivateStats()
{
    ELogType eLogType = this->GetId();
    switch (eLogType)
    {
    case LOG_TYPE_NONE:
        Statistics()->AddConsolePrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_TRACE:
        Statistics()->AddTracePrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_DEBUG:
        Statistics()->AddDebugPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_INFO:
        Statistics()->AddInfoPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_WARN:
        Statistics()->AddWarnPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_ERROR:
        Statistics()->AddErrorPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_FATAL:
        Statistics()->AddFatalPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_MAIN:
        Statistics()->AddMainPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_REMOTE:
        Statistics()->AddRemotePrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    case LOG_TYPE_SYS:
        Statistics()->AddSysPrivateQueue(static_cast<uint32_t>(m_lstPrivMessage.size()));
        break;
    default:
        break;
    }
}

/**
* @brief Run function of NamedThread.
*/
void CYLoggerBaseAppender::Run()
{
    while (IsRunning())
    {
        if (m_objTimeElapsed.Elapsed() < 100 && m_lstPublicMessage.empty())
        {
            m_objCondition.WaitForEvent(static_cast<unsigned>(abs(100 - m_objTimeElapsed.Elapsed())), [&]()->bool {
                return m_lstPublicMessage.size() > 10;
                });
        }
        m_objTimeElapsed.Reset();

        // flip double buffering.
        FlipBuffer();

        if (!m_bActivate)
        {
            EXCEPTION_BEGIN
            {
                this->OnActivate();
                m_bActivate = true;
            }
                EXCEPTION_END
        }

        // Coroutine log processing.
        auto objTuple = DoWork([&] {
            int nTotalLine = 0;
            int nTotalBytes = 0;
            while (!m_lstPrivMessage.empty())
            {
                SharePtr<CYBaseMessage> ptrMessage = m_lstPrivMessage.front();
                m_lstPrivMessage.pop_front();

                nTotalLine++;
                const TString&& strMsg = ptrMessage->GetFormatMessage();
                nTotalBytes += static_cast<int>(strMsg.size() + TEXT_BYTE_LEN);
                Log(strMsg, ptrMessage->GetTypeIndex(), false);

                UpdatePrivateStats();
            }
            return  std::tuple<int, int>(nTotalLine, nTotalBytes);
            });

        // Get result.
        EXCEPTION_BEGIN
        {
            Statistics()->AddTotalLine(get<0>(objTuple));
            Statistics()->AddTotalBytes(get<1>(objTuple));
        }
            EXCEPTION_END
    }

    this->Flush();
}

CYLOGGER_NAMESPACE_END
