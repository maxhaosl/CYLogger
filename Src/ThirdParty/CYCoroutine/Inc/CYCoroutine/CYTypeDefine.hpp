#ifndef __CY_TYPE_DEFINE_CORO_HPP__
#define __CY_TYPE_DEFINE_CORO_HPP__

#include <string>
#include <iostream>
#include <iosfwd>
#include <fstream>

#include <memory>
#include <mutex>
#include <functional>

#ifdef _WIN32
#include <direct.h>
#endif

#if defined(_WIN32)
#ifdef _UNICODE
#define CY_USE_UNICODE  1
#else
#define CY_USE_UNICODE  0
#endif
#endif

// macro
//////////////////////////////////////////////////////////////////////////
#define WIDEN2(x)		L ## x
#define WIDEN(x)		WIDEN2(x)
#define __WFILE__		WIDEN(__FILE__)
#define __WDATE__		WIDEN(__DATE__)
#define __WTIME__		WIDEN(__TIME__)

#if defined(_WIN32) && CY_USE_UNICODE
typedef std::wstring			TString;
typedef std::wstring_view       TStringView;
typedef std::wofstream			TOfStream;
typedef	std::wostringstream		TOStringStream;
typedef std::wios				TIos;
typedef std::wostream			TOStream;

typedef std::wifstream			TIfStream;
typedef std::wofstream			TOfStream;
typedef std::wstringstream		TStringStream;

#define cy_csrchr               wcsrchr
#define cy_strlen               wcslen
#define cy_vscprintf            _vscwprintf
#define cy_vsnprintf_s          _vsnwprintf_s
#define cy_mkdir				_wmkdir
#define cy_splitpath   		    _wsplitpath
#define cy_strcpy				wcscpy
#define cy_fullpath				_wfullpath
#define cy_sprintf_s			swprintf_s
#define cy_strcat_s				wcscat_s
#define cy_tcscpy_s             wcscpy_s
#define cy_tcstok_s             wcstok_s
#define cy_tcscmp               wcscmp
#define cy_strstr               wcsstr
#else
typedef std::string				TString;
typedef std::string_view        TStringView;
typedef std::ofstream			TOfStream;
typedef	std::ostringstream		TOStringStream;
typedef std::ios				TIos;
typedef std::ostream			TOStream;

typedef std::ifstream			TIfStream;
typedef std::ofstream			TOfStream;
typedef std::stringstream		TStringStream;

#define cy_csrchr               strrchr
#define cy_strlen               strlen
#define cy_vscprintf            _vscprintf
#define cy_vsnprintf_s          _vsnprintf_s
#define cy_mkdir				_mkdir
#define cy_splitpath   		    _splitpath
#define cy_strcpy				strcpy
#define cy_fullpath				_fullpath
#define cy_sprintf_s			sprintf_s
#define cy_strcat_s				strcat_s
#define cy_tcscpy_s             strcpy_s
#define cy_tcstok_s             strtok_s
#define cy_tcscmp               strcmp
#define cy_strstr               strstr
#endif

#define SharePtr				std::shared_ptr
#define UniquePtr				std::unique_ptr
#define WeakPtr					std::weak_ptr
#define MakeShared			    std::make_shared
#define MakeUnique			    std::make_unique
#define MakeTuple				std::make_tuple

using UniqueLock				= std::unique_lock<std::mutex>;
using LockGuard					= std::lock_guard<std::mutex>;
using FuncThreadDelegate		= std::function<void(std::string_view thread_name)>;

#define UNKNOWN_SEVER_CODE		-1

#if defined(_WIN32) && CY_USE_UNICODE
typedef wchar_t          TChar;
#define __TFILE__		__WFILE__
#define __TDATE__		__WDATE__
#define __TTIME__		__WTIME__
#define __TFUNCTION__	__FUNCTIONW__
#define __TLINE__		__LINE__
#else
typedef char            TChar;
#define __TFILE__		__FILE__
#define __TDATE__		__DATE__
#define __TTIME__		__TIME__
#define __TLINE__		__LINE__
#ifdef _WIN32
#define __TFUNCTION__	__func__
#else
#define __TFUNCTION__	__FUNCTION__
#endif
#endif

#ifndef TEXT
#if defined(_WIN32) && CY_USE_UNICODE
#define __TEXT(quote) L##quote 
#define TEXT_BYTE_LEN 2
#else
#define __TEXT(quote) quote    
#define TEXT_BYTE_LEN 1
#endif

#define TEXT(quote) __TEXT(quote)  
#endif

#endif //__CY_TYPE_DEFINE_CORO_HPP__