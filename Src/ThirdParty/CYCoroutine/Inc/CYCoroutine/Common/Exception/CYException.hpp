#ifndef __CY_EXCEPTION_HPP__
#define __CY_EXCEPTION_HPP__

#include "CYCoroutine/Common/Exception/CYBaseException.hpp"

CYCOROUTINE_NAMESPACE_BEGIN

static constexpr int K_LOG_LEVEL_UNKNOWN = 0;

template<int _LEVEL = K_LOG_LEVEL_UNKNOWN, int _SEVER_CODE = UNKNOWN_SEVER_CODE>
class CYException : public CYBaseException
{
public:
	/**
	* @brief The level of the exception
	*/
	static const int LEVEL = _LEVEL;

	/**
	* @brief The severity code of the exception
	*/
	static const int SEVER_CODE = _SEVER_CODE;

	/**
	* @brief Constructor.
	*/
	CYException(const TString& strMsg, const TString strFile, const TString& strFunction, int nLine) :
		CYBaseException(_LEVEL, _SEVER_CODE, strMsg, strFile, strFunction, nLine)
	{
	}

	/**
	* @brief Constructor.
	*/
	CYException(const TString& strMsg, const CYBaseException& objCause, const TString strFile, const TString& strFunction, int nLine) :
		CYBaseException(_LEVEL, _SEVER_CODE, strMsg, objCause, strFile, strFunction, nLine)
	{
	}

	/*
	* @brief Helper to throw exception if condition is true.
	*/
	inline static void IfTrueThrow(bool bCondition, const TString& strMsg, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/)
	{
		if (!bCondition)
			return;

		throw new CYException(strMsg, strFile, strFunction, nLine);
	}

	/*
	* @brief Helper to throw exception if condition is true
	*/
	inline static void IfTrueReThrow(bool bCondition, const TString& strMsg, const CYBaseException& objCause, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/)
	{
		if (!bCondition)
			return;

		throw new CYException(strMsg, objCause, strFile, strFunction, nLine);
	}

	/*
	* @brief Helper to throw exception if condition is false.
	*/
	inline static void IfFalseThrow(bool bCondition, const TString& strMsg, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/)
	{
		if (bCondition)
			return;

		throw new CYException(strMsg, strFile, strFunction, nLine);
	}

	/*
	* @brief Helper to throw exception if condition is false
	*/
	inline static void IfFalseReThrow(bool bCondition, const TString& strMsg, const CYBaseException& objCause, const TString strFile/* = __TFILE__*/, const TString& strFunction/* = __TFUNCTION__*/, int nLine/* = __TLINE__*/)
	{
		if (bCondition)
			return;

		throw new CYException(strMsg, objCause, strFile, strFunction, nLine);
	}
};

CYCOROUTINE_NAMESPACE_END

#define IfTrueThrow(bCondition, strMsg) CYCOROUTINE_NAMESPACE::CYException<K_LOG_LEVEL_UNKNOWN>::IfTrueThrow(bCondition, strMsg, __TFILE__, __TFUNCTION__, __TLINE__)
#define IfTrueReThrow(bCondition, strMsg, objCause)  CYCOROUTINE_NAMESPACE::CYException<K_LOG_LEVEL_UNKNOWN>::IfTrueReThrow(bCondition, strMsg, objCause, __TFILE__, __TFUNCTION__, __TLINE__)
#define IfFalseThrow(bCondition, strMsg) CYCOROUTINE_NAMESPACE::CYException<K_LOG_LEVEL_UNKNOWN>::IfFalseThrow(bCondition, strMsg, __TFILE__, __TFUNCTION__, __TLINE__)
#define IfFalseReThrow(bCondition, strMsg, objCause)  CYCOROUTINE_NAMESPACE::CYException<K_LOG_LEVEL_UNKNOWN>::IfFalseReThrow(bCondition, strMsg, objCause, __TFILE__, __TFUNCTION__, __TLINE__)

#endif //__CY_EXCEPTION_HPP__