#include "Src/Entity/CYLoggerEntityFactory.hpp"
#include "Src/Entity/Appender/CYLoggerBaseAppender.hpp"
#include "Src/Entity/CYLoggerEntity.hpp"
#include "Src/Entity/Appender/CYLoggerAppenderFactory.hpp"

#include <assert.h>

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief Sington.
*/
SharePtr<CYLoggerEntityFactory> CYLoggerEntityFactory::m_ptrInstance;

/**
 * @brief Map of Logger Entity.
*/
CYLoggerEntityFactory::RegisterEntity CYLoggerEntityFactory::m_mapRegisterEntity;


CYLoggerEntityFactory::CYLoggerEntityFactory()
{

}

CYLoggerEntityFactory::~CYLoggerEntityFactory()
{

}

/**
 * @brief Create Entity.
*/
SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> CYLoggerEntityFactory::CreateEntity(ELogType eLogType, const TString& strFileName, ELogFileMode eFileMode)
{
    auto iterFind = m_mapRegisterEntity.find(eLogType);
    if (iterFind != m_mapRegisterEntity.end())
    {
        return iterFind->second;
    }
    return RegisterLoggerEntity(eLogType, strFileName, eFileMode);
}

/**
 * @brief Register Logger Entity.
*/
SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> CYLoggerEntityFactory::RegisterLoggerEntity(ELogType eLogType, const TString& strFileName, ELogFileMode eFileMode)
{
    auto ptrEntity = MakeShared<CYLoggerEntity<CYLoggerBaseAppender>>(eLogType);
    auto ptrAppender = CYLoggerAppenderFactory::CreateFileAppender(strFileName, eFileMode, eLogType);
    ptrEntity->AttachAppender(ptrAppender);

    m_mapRegisterEntity[eLogType] = ptrEntity;
    return ptrEntity;
}

/**
 * @brief Get Logger Entity.
*/
SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> CYLoggerEntityFactory::GetLoggerEntity(ELogType eLogType)
{
    auto iterFind = m_mapRegisterEntity.find(eLogType);
    if (iterFind != m_mapRegisterEntity.end())
    {
        return iterFind->second;
    }
    return nullptr;
}

/**
 * @brief Release Logger Entity.
*/
void CYLoggerEntityFactory::ReleaseLoggerEntity(ELogType eLogType)
{
    auto iterFind = m_mapRegisterEntity.find(eLogType);
    if (iterFind != m_mapRegisterEntity.end())
    {
        auto ptrLoggerEntity = iterFind->second;
        ptrLoggerEntity->Flush();
        ptrLoggerEntity.reset();
        m_mapRegisterEntity.erase(eLogType);
    }
}

/**
 * @brief Release All Logger Entity.
*/
void CYLoggerEntityFactory::ReleaseAllLoggerEntity()
{
    auto iter = m_mapRegisterEntity.begin();
    while (iter != m_mapRegisterEntity.end())
    {
        auto ptrLoggerEntity = iter->second;
        ptrLoggerEntity->Flush();
        ptrLoggerEntity.reset();
        iter = m_mapRegisterEntity.erase(iter);
    }
}

/**
 * @brief Force new log file.
*/
void CYLoggerEntityFactory::ForceEntityNewFile()
{
    auto iter = m_mapRegisterEntity.begin();
    while (iter != m_mapRegisterEntity.end())
    {
        auto ptrLoggerEntity = iter->second;
        ptrLoggerEntity->ForceNewFile();
    }
}

/**
 * @brief Get CYLoggerEntityFactory Instance.
*/
std::shared_ptr<CYLoggerEntityFactory> CYLoggerEntityFactory::GetInstance()
{
    if (!m_ptrInstance)
    {
        m_ptrInstance = MakeShared<CYLoggerEntityFactory>();
    }
    return m_ptrInstance;
}

/**
 * @brief Release CYLoggerEntityFactory Instance.
*/
void CYLoggerEntityFactory::ReleaseInstance()
{
    m_ptrInstance.reset();
}

CYLOGGER_NAMESPACE_END