#include "Src/Entity/Layout/CYLoggerTemplateLayoutManager.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayoutCustom.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayout1.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayout2.hpp"
#include "Src/Entity/Layout/CYLoggerTemplateLayout3.hpp"
#include "Src/Config/CYLoggerConfig.hpp"

#include <memory>

CYLOGGER_NAMESPACE_BEGIN

/**
 * @brief Singleton.
*/
SharePtr<CYLoggerTemplateLayoutManager> CYLoggerTemplateLayoutManager::m_ptrInstance;


/**
 * @brief Get Singleton Instance.
*/
SharePtr<CYLoggerTemplateLayoutManager> CYLoggerTemplateLayoutManager::GetInstance()
{
    if (!m_ptrInstance)
    {
        m_ptrInstance = MakeShared<CYLoggerTemplateLayoutManager>();
    }
    return m_ptrInstance;
}

/**
 * @brief Free Singleton Instance.
*/
void CYLoggerTemplateLayoutManager::FreeInstance()
{
    m_ptrInstance.reset();
}

//////////////////////////////////////////////////////////////////////////
CYLoggerTemplateLayoutManager::CYLoggerTemplateLayoutManager()
    : m_eCurrentLayoutType(ELogLayoutType::LOG_LAYOUT_TYPE_BUILDIN_1)
{

}

CYLoggerTemplateLayoutManager::~CYLoggerTemplateLayoutManager()
{

}

/**
 * @brief Get Template Layout.
*/
SharePtr<ICYLoggerTemplateLayout> CYLoggerTemplateLayoutManager::GetTemplateLayout()
{
    CraeteTemplateLayout();

    return m_ptrCurrentLayout;
}

/**
 * @brief Set Custom Layout.
*/
void CYLoggerTemplateLayoutManager::SetCustomLayout(ICYLoggerTemplateLayout* pFilter)
{
    if (nullptr == pFilter)
    {
        return;
    }

    if (!m_ptrCustomLayout)
    {
        m_ptrCustomLayout = MakeShared<CYLoggerTemplateLayoutCustom>();
    }
    std::dynamic_pointer_cast<CYLoggerTemplateLayoutCustom>(m_ptrCustomLayout)->SetCustomLayout(pFilter);
}

/**
 * @brief Create Template Layout.
*/
SharePtr<ICYLoggerTemplateLayout> CYLoggerTemplateLayoutManager::CraeteTemplateLayout()
{
    if (!m_ptrCurrentLayout || m_eCurrentLayoutType!= LoggerConfig()->GetLayoutType())
    {
        m_eCurrentLayoutType = LoggerConfig()->GetLayoutType();
        switch (m_eCurrentLayoutType)
        {
        case LOG_LAYOUT_TYPE_CUSTOM:
            m_ptrCurrentLayout = m_ptrCustomLayout;
            break;
        case LOG_LAYOUT_TYPE_BUILDIN_1:
            m_ptrCurrentLayout = MakeShared<CYLoggerTemplateLayout1>();
            break;
        case LOG_LAYOUT_TYPE_BUILDIN_2:
            m_ptrCurrentLayout = MakeShared<CYLoggerTemplateLayout2>();
            break;
        case LOG_LAYOUT_TYPE_BUILDIN_3:
            m_ptrCurrentLayout = MakeShared<CYLoggerTemplateLayout3>();
            break;
        default:
            break;
        }

        // The custom template is not set, only type is set, use template 1.
        if (!m_ptrCurrentLayout)
        {
            m_ptrCurrentLayout = MakeShared<CYLoggerTemplateLayout1>();
        }
    }

    return m_ptrCurrentLayout;
}

CYLOGGER_NAMESPACE_END