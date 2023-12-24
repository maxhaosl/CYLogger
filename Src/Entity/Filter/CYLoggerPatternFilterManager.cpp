#include "Src/Entity/Filter/CYLoggerPatternFilterManager.hpp"
#include "Src/Entity/Filter/CYLoggerPatternFilterChain.hpp"

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief Singleton.
*/
SharePtr<CYLoggerPatternFilterManager> CYLoggerPatternFilterManager::m_ptrInstance;

/**
 * @brief Get Singleton Instance.
*/
SharePtr<CYLoggerPatternFilterManager> CYLoggerPatternFilterManager::GetInstance()
{
    if (!m_ptrInstance)
    {
        m_ptrInstance = MakeShared<CYLoggerPatternFilterManager>();
    }
    return m_ptrInstance;
}

/**
 * @brief Free Singleton Instance.
*/
void CYLoggerPatternFilterManager::FreeInstance()
{
    m_ptrInstance.reset();
}

/**
 * @brief Constructor.
*/
CYLoggerPatternFilterManager::CYLoggerPatternFilterManager()
{

}

/**
 * @brief Destructor.
*/
CYLoggerPatternFilterManager::~CYLoggerPatternFilterManager()
{

}

/**
 * @brief Get Basic Pattern Filter.
*/
SharePtr<ICYLoggerPatternFilter> CYLoggerPatternFilterManager::GetPatternFilter()
{
    if (!m_ptrHeadFilter)
    {
        m_ptrHeadFilter = MakeShared<CYLoggerPatternFilterChain>();
    }
    return m_ptrHeadFilter;
}

/**
 * @brief Set Custom Filter.
*/
void CYLoggerPatternFilterManager::SetNextFilter(ICYLoggerPatternFilter* pFilter)
{
    GetPatternFilter()->SetNextFilter(pFilter);
}

CYLOGGER_NAMESPACE_END