#include "Common/Thread/CYNamedCondition.hpp"

#include <chrono>
using namespace std::chrono;

CYLOGGER_NAMESPACE_BEGIN

/**
  * @brief Constructor.
 */
CYNamedCondition::CYNamedCondition(const TChar* pszName /*= TEXT("")*/, bool bWait /*= true*/)
	: m_bWait(bWait)
{
	if (pszName != nullptr)
	{
		cy_strcpy(m_szName, pszName);
	}
}

/**
  * @brief Destructor.
 */
CYNamedCondition::~CYNamedCondition() noexcept
{
}

/**
 * @brief Operation waiting.
*/
RetCode CYNamedCondition::WaitForEvent()
{
	UniqueLock locker(m_mutex);
	m_cvCondition.wait(locker, [&]() {
		return !m_bWait;
		});
	return RetCode::ERR_NOERR;
}

/**
 * @brief Operation waiting for timeout.
*/
RetCode CYNamedCondition::WaitForEvent(unsigned long nTimeoutMilliseconds, std::function<bool()>&& pred)
{
    UniqueLock locker(m_mutex);
    return m_cvCondition.wait_for(locker, milliseconds(nTimeoutMilliseconds), [&, pred=std::forward<decltype(pred)>(pred)]() {
        return !m_bWait || pred();
        }) ? RetCode::ERR_NOERR : RetCode::ERR_COND_TIMEOUT;
}

/**
 * @brief Operation waiting for timeout.
*/
RetCode CYNamedCondition::WaitForEvent(unsigned long nTimeoutMilliseconds)
{
	UniqueLock locker(m_mutex);
	return m_cvCondition.wait_for(locker, milliseconds(nTimeoutMilliseconds), [&]() {
		return !m_bWait;
		}) ? RetCode::ERR_NOERR : RetCode::ERR_COND_TIMEOUT;
}

/**
 * @brief Notify an execution.
*/
RetCode CYNamedCondition::SignalOne(bool bWait/* = false*/)
{
	LockGuard locker(m_mutex);
	m_bWait = bWait;
	m_cvCondition.notify_one();
	return RetCode::ERR_NOERR;
}

/**
 * @brief Notify all executions.
*/
RetCode CYNamedCondition::SignalAll(bool bWait/* = false*/)
{
	LockGuard locker(m_mutex);
	m_bWait = bWait;
	m_cvCondition.notify_all();
	return RetCode::ERR_NOERR;
}

/**
 * @brief Reset Condition.
*/
RetCode CYNamedCondition::Reset()
{
	LockGuard locker(m_mutex);
	m_bWait = true;
	return RetCode::ERR_NOERR;
}

CYLOGGER_NAMESPACE_END