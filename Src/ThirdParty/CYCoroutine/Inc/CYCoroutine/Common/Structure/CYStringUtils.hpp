#ifndef __CY_STRING_UTILS_HPP__
#define __CY_STRING_UTILS_HPP__

#include "CYCoroutine/CYCoroutineDefine.hpp"

#include <string>

CYCOROUTINE_NAMESPACE_BEGIN

class CYStringUtils
{
public:
	/**
	 * @brief wstring convert to string.
	*/
    static std::string WString2String(const std::wstring strSrc);

	/**
	 * @brief string convert to wstring.
	*/
    static std::wstring String2WString(const std::string strSrc);

#ifdef UNICODE
	static std::wstring String2TString(const std::wstring strSrc)
	{
		return strSrc;
	}

    static std::wstring String2TString(const std::string strSrc)
    {
		return String2WString(strSrc);
    }

    static std::string TString2String(const std::wstring strSrc)
    {
        return WString2String(strSrc);
    }

    static std::string TString2String(const std::string strSrc)
    {
        return strSrc;
    }
#else
    static std::string String2TString(const std::wstring strSrc)
    {
        return WString2String(strSrc);
    }

    static std::string String2TString(const std::string strSrc)
    {
        return strSrc;
    }

    static std::string TString2String(const std::wstring strSrc)
    {
        return WString2String(strSrc);
    }

    static std::string TString2String(const std::string strSrc)
    {
        return strSrc;
    }
#endif
};

#define AtoT(x) CYStringUtils::String2TString(x).c_str()

CYCOROUTINE_NAMESPACE_END

#endif // __CY_STRING_UTILS_HPP__