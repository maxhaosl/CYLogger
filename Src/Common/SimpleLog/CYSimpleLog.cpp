#include "Src/Common/SimpleLog/CYSimpleLog.hpp"
#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"

#include <sys/stat.h>
#include <tchar.h>
#include <format>

CYLOGGER_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////

/**
 * @brief Convert Log Path.
*/
static const TString ConvertFilePath(const TChar* pszFileName, const TChar* pszLogPath, const TChar* pszLogDir = TEXT("Log"))
{
	if (pszLogDir && cy_strlen(pszLogDir) > 0)
	{
		return std::format(TEXT("{}{}{}{}"), pszLogPath, pszLogDir, LOG_SEPARATOR, pszFileName);
	}
	else
	{
		return std::format(TEXT("{}{}"), pszLogPath, pszFileName);
	}
}

//////////////////////////////////////////////////////////////////////////
CYSimpleLog::CYSimpleLog() 
: m_eLogType(LOG_TYPE_NONE)
, m_bInit(false)
, m_bLogTime(false)
, m_bLogLineCount(false)
, m_nLogLineCount(0)
, m_dwLineCount(1)
{ /* Nothing */
}

CYSimpleLog::~CYSimpleLog()
{ /* Nothing */
}

bool CYSimpleLog::WriteLog(const TChar* szStr, ...)
{
	if (szStr == nullptr)
		return false;

	if (cy_strlen(szStr) == 0)
		return false;

	if (!m_bInit)
		return false;

	va_list args;
	int iLen = 0;
	TChar szString[MAX_LOG_LINE_SIZE+1] = { 0, };
	TChar mix_string[MAX_LOG_LINE_SIZE+1] = { 0, };
	TChar szLine[10+1] = { 0, };
	TChar szDate[128+1] = { 0, };

	if (m_bLogLineCount)
	{
		cy_sprintf_s(szLine, 10, TEXT("%07d"), m_nLogLineCount++);
	}

	if (m_bLogTime)
	{
		CYTimeStamps t;
		t.SetTime();

		cy_sprintf_s(szDate, 128, TEXT("[%04d-%02d-%02d %02d:%02d:%02d] : "),
			t.GetYY(),
			t.GetMM(),
			t.GetDD(),
			t.GetHR(),
			t.GetMN(),
			t.GetSC());
	}

	va_start(args, szStr);
	iLen = _vsctprintf(szStr, args) + 1;
	_vstprintf_s(szString, iLen, szStr, args);
	va_end(args);

	if (m_bLogLineCount || m_bLogTime)
		cy_sprintf_s(mix_string, MAX_LOG_LINE_SIZE, TEXT("%s - %s%s\n"), szLine, szDate, szString);
	else
		cy_sprintf_s(mix_string, MAX_LOG_LINE_SIZE, TEXT("%s\n"), szString);

	return WriteString(mix_string);
};

//////////////////////////////////////////////////////////////////////////
CYSimpleLogFile::CYSimpleLogFile() 
: m_pFile(nullptr)
{
	m_eLogType = LOG_TYPE_FILE;

	ZeroMemory(m_szFilePath, sizeof(TChar) * MAX_LOG_PATH_SIZE);
}
CYSimpleLogFile::~CYSimpleLogFile()
{
	CloseLog();
}

bool CYSimpleLogFile::InitLog(bool bLogTime, bool bLogLineCount, const TChar* szWorkPath, const TChar* pszLogDir, const TChar* szFilePath, va_list args)
{
	if (m_bInit)
		return true;

	if (szFilePath == nullptr)
		return false;

	if (_tcsclen(szFilePath) == 0)
		return false;

	if (_tcsclen(szFilePath) >= MAX_LOG_PATH_SIZE)
		return false;

	m_bLogTime = bLogTime;
	m_bLogLineCount = bLogLineCount;

	TChar szPath[MAX_LOG_PATH_SIZE];

	va_list args_copy;

#if defined(_MSC_VER) || defined(__BORLANDC__)
	args_copy = args;
#else
	va_copy(args_copy, args);
#endif
	//va_start(args, szFilePath);
	_vstprintf_s(szPath, szFilePath, args_copy);
	va_end(args_copy);

	cy_tcscpy_s(m_szFilePath, MAX_LOG_PATH_SIZE, ConvertFilePath(szPath, szWorkPath, pszLogDir).c_str()); 
	cy_tcscpy_s(m_szWorkPath, MAX_LOG_PATH_SIZE, szWorkPath);
	cy_tcscpy_s(m_szLogDir, MAX_LOG_PATH_SIZE, pszLogDir);
	
	CreateDir();

	m_bInit = true;

	return true;
}

void CYSimpleLogFile::CreateDir()
{
	if (m_szFilePath == nullptr)
		return;

	if (0 == cy_strlen(m_szFilePath))
		return;

	TChar seps[] = LOG_SEPARATOR_STR;
	TChar* token, * next_token;
	TChar szTemp[MAX_LOG_PATH_SIZE], szDir[MAX_LOG_PATH_SIZE];

	cy_tcscpy_s(szTemp, MAX_LOG_PATH_SIZE, m_szFilePath);

	memset(szDir, 0, sizeof(TChar) * MAX_LOG_PATH_SIZE);

	token = cy_tcstok_s(szTemp, seps, &next_token);

	while ((token != nullptr))
	{
		if (token[0] != '.')
		{
			if (nullptr != _tcschr(token, '.'))
				break;

			if (0 != cy_strlen(szDir))
				cy_strcat_s(szDir, MAX_LOG_PATH_SIZE, LOG_SEPARATOR_STR);

			cy_strcat_s(szDir, MAX_LOG_PATH_SIZE, token);
			cy_mkdir(szDir);
		}
		else
			cy_strcat_s(szDir, MAX_LOG_PATH_SIZE, token);

		token = cy_tcstok_s(nullptr, seps, &next_token);
	}
}

void CYSimpleLogFile::CloseLog()
{
	if (m_bInit && m_pFile != nullptr)
	{
		fclose(m_pFile);
	}

	m_bInit = false;
}

bool CYSimpleLogFile::WriteString(TChar* szStr)
{
	try
	{
        errno_t err;

#if CY_USE_UNICODE
        if ((err = _tfopen_s(&m_pFile, m_szFilePath, TEXT("at,ccs=UNICODE"))) != 0)
        {
            if (ENOENT == err)
            {
                CreateDir();
                if ((err = _tfopen_s(&m_pFile, m_szFilePath, TEXT("at,ccs=UNICODE"))) != 0)
                    return false;
            }
            else
                return false;
        }
#else	//CY_USE_UNICODE
        if ((err = _tfopen_s(&m_pFile, m_szFilePath, TEXT("at"))) != 0)
        {
            if (ENOENT == err)
            {
                CreateDir();
                if ((err = _tfopen_s(&m_pFile, m_szFilePath, TEXT("at"))) != 0)
                    return false;
            }
            else
                return false;
        }
#endif	//CY_USE_UNICODE

        if (nullptr == m_pFile)
            return false;

        if (m_pFile && fseek(m_pFile, 0, SEEK_END) != 0)
        {
            if(m_pFile) fclose(m_pFile);
            return false;
        }

#if CY_USE_UNICODE
		if (m_pFile)
		{
            long Filesize = ftell(m_pFile);
            if (0 == Filesize)
            {
                BYTE BOM[] = { 0xff, 0xfe };
                if (m_pFile) fwrite(BOM, 2, 1, m_pFile);
                if (m_pFile) fclose(m_pFile);
                return WriteString(szStr);
            }
		}
#endif	//CY_USE_UNICODE

		int nLength = cy_strlen(szStr);
		if (nLength > 150)
		{
			 nLength = cy_strlen(szStr);
		}

        if(m_pFile) fwrite(szStr, sizeof(TChar), nLength, m_pFile);
        if(m_pFile) fclose(m_pFile);

        return true;
	}
	catch (...)
	{
		
	}
	 if(m_pFile) fclose(m_pFile);
	 m_pFile = nullptr;
	return false;
}

void CYSimpleLogFile::DeleteAllFile(TChar* _szDir)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TChar strDir[MAX_LOG_PATH_SIZE];
	TChar strName[MAX_LOG_PATH_SIZE];
	TChar strFile[MAX_LOG_PATH_SIZE];

	cy_tcscpy_s(strDir, MAX_LOG_PATH_SIZE, TEXT("\\"));
	cy_strcat_s(strDir, MAX_LOG_PATH_SIZE, _szDir);
	cy_tcscpy_s(strName, MAX_LOG_PATH_SIZE, ConvertFilePath(strDir, m_szWorkPath, m_szLogDir).c_str()); 
	cy_strcat_s(strName, TEXT("\\*"));

	hFind = FindFirstFile(strName, &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			if (FILE_ATTRIBUTE_DIRECTORY == FindFileData.dwFileAttributes)
			{
				if (0 != cy_tcscmp(FindFileData.cFileName, TEXT(".")) && 0 != cy_tcscmp(FindFileData.cFileName, TEXT("..")))
				{
					cy_tcscpy_s(strFile, MAX_LOG_PATH_SIZE, _szDir);
					cy_strcat_s(strFile, TEXT("\\"));
					cy_strcat_s(strFile, FindFileData.cFileName);
					DeleteAllFile(strFile);
				}
				continue;
			}

			cy_tcscpy_s(strFile, MAX_LOG_PATH_SIZE, ConvertFilePath(strDir, m_szWorkPath, m_szLogDir).c_str()); 
			cy_strcat_s(strFile, TEXT("\\"));
			cy_strcat_s(strFile, FindFileData.cFileName);
			DeleteFile(strFile);
		}
		FindClose(hFind);
	}
}

//////////////////////////////////////////////////////////////////////////
CYSimpleLogConsole::CYSimpleLogConsole() 
: m_hInputHandle(INVALID_HANDLE_VALUE)
, m_hOutputHandle(INVALID_HANDLE_VALUE)
{
	m_eLogType = LOG_TYPE_CONSOLE;

	ZeroMemory(m_szConsoleTitle, sizeof(TChar) * MAX_CONSOLE_TITLE_SIZE);
}

CYSimpleLogConsole::~CYSimpleLogConsole()
{
	CloseLog();
}

bool CYSimpleLogConsole::InitLog(bool bLogTime, bool bLogLineCount, const TChar* szWorkPath/* = nullptr*/, const TChar* pszLogDir/* = TEXT("Log")*/, const TChar* szConsoleTitle, va_list args)
{
	m_bLogTime = bLogTime;
	m_bLogLineCount = bLogLineCount;

	va_list args_copy;

#if defined(_MSC_VER) || defined(__BORLANDC__)
	args_copy = args;
#else
	va_copy(args_copy, args);
#endif
	//va_start(args, szFilePath);
	cy_vsnprintf_s(m_szConsoleTitle, 128, 128, szConsoleTitle, args_copy);
	va_end(args_copy);

	if (!szConsoleTitle)
		cy_strcpy(m_szConsoleTitle, DEFAULT_CONSOLE_TOTLE);

	AllocConsole();
	SetConsoleTitle(m_szConsoleTitle);

	m_hInputHandle = GetStdHandle(STD_INPUT_HANDLE);
	m_hOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (INVALID_HANDLE_VALUE == m_hInputHandle || INVALID_HANDLE_VALUE == m_hOutputHandle)
		return false;

	m_bInit = true;

	return true;
}

void CYSimpleLogConsole::CloseLog()
{
	if (!m_bInit)
		return;

	FreeConsole();
}

bool CYSimpleLogConsole::WriteString(TChar* szStr)
{
	if (!m_bInit)
		return false;

	if (INVALID_HANDLE_VALUE == m_hOutputHandle)
		return false;

	DWORD dwWrittenSize = 0;
    WriteConsole(m_hOutputHandle, szStr, (unsigned long)cy_strlen(szStr), &dwWrittenSize, nullptr);
    OutputDebugString(szStr);
    OutputDebugString(TEXT("\n"));

	return true;
}

CYLOGGER_NAMESPACE_END