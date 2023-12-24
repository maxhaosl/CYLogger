#ifndef __CY_BASE_EXCEPTION_HPP__
#define __CY_BASE_EXCEPTION_HPP__

#include "CYCoroutine/Common/Message/CYBaseMessage.hpp"
#include "CYCoroutine/Common/Structure/CYStringUtils.hpp"

#include <io.h>
#include <string>
#include <sstream>

CYCOROUTINE_NAMESPACE_BEGIN

class CYBaseException : public std::exception, public CYBaseMessage
{
public:
    /**
    * @brief Constructor.
    */
    CYBaseException(const CYBaseMessage& strMsg);

    /**
    * @brief Constructor.
    */
    CYBaseException(const CYBaseMessage& strMsg, const CYBaseException& objCause);

    /**
    * @brief Constructor.
    */
    CYBaseException(int nType, int nServerCode, const TString& strMsg, const TString strFile, const TString& strFunction, int nLine);

    /**
    * @brief Constructor.
    */
    CYBaseException(int nType, int nServerCode, const TString& strMsg, const CYBaseException& cause, const TString strFile, const TString& strFunction, int nLine);

    /**
     * @brief Destructor.
    */
    virtual ~CYBaseException() noexcept = default;

    /**
    * @brief Inherited from std::exception
    */
    virtual char const* what() const override;

public:
    /**
    * Inherited from CYBaseMessage, the result is in the following format: [YYYYMMDD hh:mm:ss.nnnnnn |{T,D,I,W,E,F,U}: severCode] location message [(errno=x system message)] [***caused by***] \r\n
    * @param strMsg a reference where the result (formatted message, explaining the exception) will be put
    * @return strMsg.c_str()
    */
    virtual const TString GetFormatMessage() const override;

    /**
     * @brief Get Type Index
    */
    virtual int32_t GetTypeIndex() override;

    /**
     * @brief Get Caused by.
    */
    inline const CYBaseException* GetCause() const;

    /*
    * Helper to throw exception if condition is true
    */
    inline static void IfTrueThrow(bool bCondition, const CYBaseMessage& strMsg);

    /*
    * Helper to throw exception if condition is false
    */
    inline static void IfFalseThrow(bool bCondition, const CYBaseMessage& strMsg);

    /*
    * Helper to throw exception if condition is true
    */
    inline static void IfTrueThrow(bool bCondition, const CYBaseMessage& strMsg, const CYBaseException& objCause);

    /*
    * Helper to throw exception if condition is false
    */
    inline static void IfFalseThrow(bool bCondition, const CYBaseMessage& strMsg, const CYBaseException& objCause);

    /*
    * Helper to throw exception if condition is true.
    */
    inline static void IfTrueThrow(bool bCondition, int nType, int nServerCode, const TString& strMsg, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/);

    /*
    * Helper to throw exception if condition is false.
    */
    inline static void IfFalseThrow(bool bCondition, int nType, int nServerCode, const TString& strMsg, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/);

    /*
    * Helper to throw exception if condition is true
    */
    inline static void IfTrueThrow(bool bCondition, int nType, int nServerCode, const TString& strMsg, const CYBaseException& objCause, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/);

    /*
    * Helper to throw exception if condition is false
    */
    inline static void IfFalseThrow(bool bCondition, int nType, int nServerCode, const TString& strMsg, const CYBaseException& objCause, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/);

private:
    int                         m_nTypeIndex = 0;
    int							m_nErrNo = 0;
    mutable TString				m_strFormatMsg;
    TString				        m_strMsg;
    mutable std::string			m_strWhat;
    SharePtr<CYBaseException>	m_ptrCause;
};

CYCOROUTINE_NAMESPACE_END

#endif //__CY_BASE_EXCEPTION_HPP__