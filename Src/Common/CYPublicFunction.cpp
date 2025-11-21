#include "Common/CYPublicFunction.hpp"
#include "CYCoroutine/Common/Exception/CYException.hpp"
#include "Common/CYPrivateDefine.hpp"
#include "Common/Thread/CYNamedLocker.hpp"
#include "Common/CYFormatDefine.hpp"
#include "CYCoroutine/Common/Time/CYTimeStamps.hpp"
#ifdef CYLOGGER_WIN_OS
#include <io.h>
#include <process.h>
#include <filesystem>
#include <sys/types.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <algorithm>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdio>
#include <chrono>

#ifdef CYLOGGER_WIN_OS
#pragma warning(disable:4311 4302)
#endif

CYLOGGER_NAMESPACE_BEGIN

/**
* Extracts the directory name from path and deeply creates it
*/
void CYPublicFunction::CreateDirectory(const TString& strPath)
{
    IfFalseThrow(strPath.length() != 0, TEXT("path cannot be empty"));

#ifdef CYLOGGER_WIN_OS
    TChar drive[_MAX_DRIVE + 1];
    TChar dir[_MAX_DIR + 1];
    TChar fname[_MAX_FNAME + 1];
    TChar ext[_MAX_FNAME + 1];
    TChar p[_MAX_DIR + 1];
    std::vector<TString> dirs;

    cy_splitpath(strPath.c_str(), drive, dir, fname, nullptr);
    for (int len = 0; dir != nullptr && (len = (int)cy_strlen(dir));)
    {
        dir[len - 1] = 0;
        if (cy_strlen(dir) == 0)
            break;

        cy_strcpy(p, dir);
        cy_splitpath(p, nullptr, dir, fname, ext);
        if (cy_strlen(ext) > 0)
        {
            cy_strcat_s(fname, ext);
        }
        dirs.push_back(fname);
    }
#else
    // Unix/Linux/macOS implementation
    std::vector<TString> dirs;
    TString path = strPath;
    size_t pos = 0;
    size_t lastPos = 0;

    while ((pos = path.find('/', lastPos)) != TString::npos)
    {
        if (pos > lastPos)
        {
            dirs.push_back(path.substr(0, pos));
        }
        lastPos = pos + 1;
    }
    if (lastPos < path.length())
    {
        dirs.push_back(path);
    }
#endif

    if (dirs.size() == 0)
        return;

#ifdef CYLOGGER_WIN_OS
    TString sPath;
    if (drive != nullptr && cy_strlen(drive) != 0)
    {
        sPath = drive;
        sPath += LOG_SEPARATOR;
    }
    else if (strPath[0] == LOG_SEPARATOR)
        sPath = LOG_SEPARATOR;
#else
    TString sPath;
    if (strPath[0] == LOG_SEPARATOR)
        sPath = LOG_SEPARATOR;
#endif

    for (int i = (int)dirs.size() - 1; i >= 0; --i)
    {
        sPath.append(dirs[i]).append(1, LOG_SEPARATOR);
        IfTrueThrow(cy_mkdir(sPath.c_str()) != 0 && errno != EEXIST, (TString(TEXT("mkdir() failed for ")) += sPath).c_str());
    }
}

/**
* Makes a copy of a file, creating the target file with all the directories if necessarily
*/
void CYPublicFunction::CopyFile(const TString& strSrc, const TString& strDst, bool bAppendMode, bool bFailedIfDstExists)
{
    IfFalseThrow(strSrc.length() != 0, TEXT("src file name cannot be empty"));
    IfFalseThrow(strDst.length() != 0, TEXT("dst file name cannot be empty"));

    TChar* p = cy_fullpath(nullptr, strSrc.c_str(), 0);
    IfFalseThrow(p != nullptr, TString(TEXT("src file cy_fullpath() failed for ")) += strSrc);

    TString srcFileName = p;
    delete p;
    p = cy_fullpath(nullptr, strDst.c_str(), 0);
    IfFalseThrow(p != nullptr, TString(TEXT("dst file cy_fullpath() failed ")) += strDst);

    TString dstFileName = p;
    delete p;
    std::transform(srcFileName.begin(), srcFileName.end(), srcFileName.begin(), tolower);
    std::transform(dstFileName.begin(), dstFileName.end(), dstFileName.begin(), tolower);
    IfFalseThrow(srcFileName != dstFileName, TEXT("src file cannot be a target for copy"));
    CreateDirectory(dstFileName);

    TIfStream srcFile(strSrc.c_str());
    IfTrueThrow(!srcFile, TString(TEXT("src file does not exist or open error occured ")) += strSrc);

    TOfStream dstFile;
    if (bFailedIfDstExists)
    {
        dstFile.open(strDst.c_str(), TIos::in | TIos::out);
        IfTrueThrow(dstFile.is_open(), TString(TEXT("dst file already exists ")) += strDst);
        dstFile.close();
    }

    dstFile.open(strDst.c_str(), bAppendMode ? TIos::out | TIos::app : TIos::out);
    IfTrueThrow(!dstFile.is_open(), TString(TEXT("dst file open/creation error ")) += strDst);
    if (bAppendMode)
    {
        dstFile.seekp(0, std::ios_base::end);
        IfTrueThrow(dstFile.fail(), TString(TEXT("dst file seekp() failed ")) += strDst);
    }

    dstFile << srcFile.rdbuf();
    IfTrueThrow(dstFile.fail(), TString(TEXT("dst file operator<<() failed ")) += strDst);

    dstFile.flush();
    IfTrueThrow(dstFile.fail(), TString(TEXT("dst file Flush() failed ")) += strDst);
}

/**
* Synchronized console output, multithread ready
*/
void CYPublicFunction::WriteToConsole(const TString& strMsg)
{
    static CYNamedLocker lock{ TEXT("CYPublicFunction") };
    lock.lock();

    TChar szMsg[4096];
    cy_sprintf_s(szMsg, 4096, TEXT("%s\n"), strMsg.c_str());

#ifdef CYLOGGER_WIN_OS
    DWORD dwWrittenSize = 0;
    uint32_t color = TWHITE;
    switch (strMsg[26])
    {
    case 'I': color = TWHITE;	break;
    case 'D': color = TGREEN;	break;
    case 'B': color = TBLUE;	break;
    case 'W': color = TYELLOW;	break;
    case 'E': color = TRED;		break;
    case 'S': color = TRED;		break;
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), szMsg, (DWORD)cy_strlen(szMsg), &dwWrittenSize, nullptr);
#else
    // Unix/Linux/macOS - just print the message
    printf(TEXT("%s"), szMsg);
#endif

    lock.unlock();
}

/**
* Sleep millisec
*/
void CYPublicFunction::SLEEP(unsigned long millisec)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
}

/**
 * @brief Format String.
*/
const TString CYPublicFunction::FmtString(const TChar* pszFormat, ...)
{
#define	MAX_FMT_STRING	4096
    TChar string[MAX_FMT_STRING];

    va_list argptr;
    va_start(argptr, pszFormat);
#ifdef CYLOGGER_WIN_OS
    cy_vsnprintf_s(string, MAX_FMT_STRING, _TRUNCATE, pszFormat, argptr);
#else
    cy_vsnprintf_s(string, MAX_FMT_STRING, MAX_FMT_STRING, pszFormat, argptr);
#endif
    va_end(argptr);

    return string;
}

int CYPublicFunction::PrintLog(FILE* pfile, const char* pszFormat, ...)
{
    va_list _va_list;
    char szBuf[MAXBUF];
    int nLen;
    if (pszFormat == nullptr || pfile == nullptr) return -1;
    CYTimeStamps objTime;
#ifdef CYLOGGER_WIN_OS
    nLen = snprintf(szBuf, sizeof(szBuf), " %04d.%02d.%02d %02d:%02d:%02d [%d]: ",
        objTime.GetYY(), objTime.GetMM(), objTime.GetDD(),
        objTime.GetHR(), objTime.GetMN(), objTime.GetSC(), _getpid());
#else
    nLen = snprintf(szBuf, sizeof(szBuf), " %04d.%02d.%02d %02d:%02d:%02d [%d]: ",
        objTime.GetYY(), objTime.GetMM(), objTime.GetDD(),
        objTime.GetHR(), objTime.GetMN(), objTime.GetSC(), getpid());
#endif

    va_start(_va_list, pszFormat);
    nLen += vsnprintf(szBuf + nLen, sizeof(szBuf) - nLen, pszFormat, _va_list);
    va_end(_va_list);

    nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "\n");
    if (fputs(szBuf, pfile) != EOF && fflush(pfile) != EOF) return 0;
    return -2;
}

int CYPublicFunction::PrintHexLog(FILE* pfile, void* pData, int nSize)
{
#undef min
    char cLine[16], * pcData;
    char szBuf[MAXBUF + 1];
    int nPos, nLineSize, nLine, nLen, n;
    if (pfile == nullptr || pData == nullptr) return -1;

    pcData = (char*)pData;
    nLine = 1;
    nLen = snprintf(szBuf, sizeof(szBuf), "address[%08lX] size[%d]\n", (unsigned long)pData, nSize);
    for (nPos = 0; nPos < nSize; nLine++)
    {
        nLineSize = std::min(nSize - nPos, 16);
        memcpy(cLine, pcData + nPos, nLineSize);
        nPos += nLineSize;
        nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "[%02d]:  ", nLine);
        for (n = 0; n < nLineSize; n++)
        {
            if (n == 8) nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, " ");
            nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "%02X ", cLine[n] & 0x00FF);
        }
        for (n = nLineSize; n < 16; n++)
        {
            if (n == 8) nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, " ");
            nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "   ");
        }
        nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, " :");
        for (n = 0; n < nLineSize; n++)
        {
            if (!isprint(cLine[n])) cLine[n] = '.';
            nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "%c", cLine[n]);
        }
        nLen += snprintf(szBuf + nLen, sizeof(szBuf) - nLen, "\n");
    }
    PrintLog(pfile, szBuf);
    return 0;
}

int CYPublicFunction::PrintTraceLog(const char* pszFormat, ...)
{
    va_list _va_list;
    char szBuf[MAXBUF];
    FILE* fp;
    int ret;
    if (pszFormat == nullptr) return -1;

    va_start(_va_list, pszFormat);
    vsnprintf(szBuf, sizeof(szBuf), pszFormat, _va_list);
    va_end(_va_list);

    if ((fp = fopen(TRACE_FILE, "a")) != nullptr)
    {
        ret = PrintLog(fp, szBuf);
        fclose(fp);
        return(ret);
    }
    return -1;
}

int CYPublicFunction::PrintTraceHexLog(void* pData, int nSize)
{
    FILE* fp;
    int ret;
    if ((fp = fopen(TRACE_FILE, "a")) != nullptr)
    {
        ret = PrintHexLog(fp, pData, nSize);
        fclose(fp);
        return(ret);
    }
    return -1;
}

int CYPublicFunction::Verify(int bStatus, const char* szBuf, const char* szFile, int nLine)
{
    FILE* fp = nullptr;
    char szFileLine[128], szError[128];
    if (!bStatus)
    {
        memset(szFileLine, 0, sizeof(szFileLine));
        memset(szError, 0, sizeof(szError));
        if (errno != 0) snprintf(szError, sizeof(szError), "\t> %.64s\n", strerror(errno));
        if (szFile == nullptr) strcpy(szFileLine, "\t> Invalid file name");
        else snprintf(szFileLine, sizeof(szFileLine), "\t> In line %d file %.32s", nLine, szFile);
        if (szBuf == nullptr)  szBuf = "";
        fp = fopen(TRACE_FILE, "a");
        if (fp != nullptr)
        {
#ifdef CYLOGGER_WIN_OS
            PrintLog(fp, "%s[%d]\n%s%s", szBuf, _getpid(), szError, szFileLine);
#else
            PrintLog(fp, "%s[%d]\n%s%s", szBuf, getpid(), szError, szFileLine);
#endif
            fclose(fp);
        }
        errno = 0;
    }
    return bStatus;
}

/**
 * @brief Read Or Write File.
*/
int CYPublicFunction::ReadFile(int nFile, void* pData, int* pSize)
{
    int nLeft, nRead;
    ASSERT(pData != nullptr && pSize != nullptr);
    char* pcData = (char*)pData;
    nLeft = *pSize;
    while (nLeft > 0)
    {
#ifdef CYLOGGER_WIN_OS
        if ((nRead = _read(nFile, pcData, nLeft)) < 0)
#else
        if ((nRead = read(nFile, pcData, nLeft)) < 0)
#endif
        {
            if (errno != EINTR) ASSERT(0);
            nRead = 0;
        }
        else if (nRead == 0) break;
        nLeft -= nRead;
        pcData += nRead;
    }
    *pSize = *pSize - nLeft;
    return 0;
}

int CYPublicFunction::WriteFile(int nFile, void* pData, int nSize)
{
    int nLeft = nSize, nWrite;
    const char* pcData = (const char*)pData;
    ASSERT(pData != nullptr);
    while (nLeft > 0)
    {
#ifdef CYLOGGER_WIN_OS
        if ((nWrite = _write(nFile, pcData, nLeft)) <= 0)
#else
        if ((nWrite = write(nFile, pcData, nLeft)) <= 0)
#endif
        {
            if (errno != EINTR) ASSERT(0);
            nWrite = 0;
        }
        nLeft -= nWrite;
        pcData += nWrite;
    }
    return 0;
}

/**
 * @brief Trim String.
*/
char* CYPublicFunction::TrimString(char* szDest)
{
    int n, nLen;
    if (szDest != nullptr)
    {
        for (n = (int)strlen(szDest); n > 0; n--)
        {
            if (!isblank(szDest[n - 1]))  break;
        }
        szDest[n] = '\0';

        nLen = (int)strlen(szDest);
        for (n = 0; n < nLen; n++)
        {
            if (!isblank(szDest[n])) break;
        }
        strcpy(szDest, szDest + n);
    }
    return szDest;
}

/**
 * @brief Trim String.
*/
TString CYPublicFunction::TrimString(const TString& s)
{
    static const TChar* pszWhiteSpace = TEXT(" \t\r\n");

    // test for null string
    if (s.empty())
        return s;

    // find first non-space character
    std::string::size_type b = s.find_first_not_of(pszWhiteSpace);
    if (b == std::string::npos) // No non-spaces
        return TEXT("");

    // find last non-space character
    std::string::size_type e = s.find_last_not_of(pszWhiteSpace);

    // return the remaining characters
    return TString(s, b, e - b + 1);
}

/**
 * @brief Get File Name.
*/
TString CYPublicFunction::GetFileName(const TString& strPath)
{
    TString strFileName = strPath;
    size_t i = strPath.rfind(LOG_SEPARATOR);
    if (i != TString::npos)
    {
        strFileName = strPath.substr(i + 1);
    }

    size_t index = TString::npos;
    index = strFileName.find_last_of(TEXT("."));

    if (index != TString::npos)
    {
        TString strLeft = strFileName.substr(0, index);
        //TString strRight = strResult.substr(index + 1);

        strFileName = strLeft;
    }

    return strFileName;
}

/**
 * @brief Get File Ext.
*/
TString CYPublicFunction::GetFileExt(const TString& strPath)
{
    TString strFileName = strPath;
    size_t i = strPath.rfind(LOG_SEPARATOR);
    if (i != TString::npos)
    {
        strFileName = strPath.substr(i + 1);
    }

    size_t index = TString::npos;
    index = strFileName.find_last_of(TEXT("."));

    if (index != TString::npos)
    {
        //TString strLeft = strFileName.substr(0, index);
        TString strRight = strFileName.substr(index + 1);

        strFileName = strRight;
    }

    return strFileName;
}

/**
 * @brief Get base log file name.
*/
TString CYPublicFunction::GetBaseLogName(const TString& strPath)
{
    TString strFileName = GetFileName(strPath);

    size_t index = TString::npos;
    index = strFileName.find_last_of(TEXT("_"));
    if (index != TString::npos)
    {
        return strFileName.substr(0, index);
    }
    return strFileName;
}

TString CYPublicFunction::GetBasePath(const TString& strPath)
{
    TString strFileName = strPath;
    TString strFilePath;
    size_t index = strPath.rfind(LOG_SEPARATOR);
    if (index != TString::npos)
    {
        strFileName = strPath.substr(index + 1);
        strFilePath = strPath.substr(0, index + 1);
    }

    index = strFileName.find_last_of(TEXT("_"));
    if (index != TString::npos)
    {
        strFileName = strFileName.substr(0, index);
    }

    return strFilePath + strFileName;
}

/**
 * @brief Remove File.
*/
bool CYPublicFunction::Remove(const TString& strPath)
{
#if defined(CYLOGGER_MAC_OS)
    // macOS 10.15 filesystem::remove is not available ¡ú use POSIX unlink
    return ::unlink(strPath.c_str()) == 0;
#elif defined(CYLOGGER_WIN_OS)
    return std::filesystem::remove(strPath) == 0;
#else
    // Windows/Linux/macOS 11+
    return std::remove(strPath.c_str()) == 0;
#endif
}

/**
 * @brief Get File SIze.
 */
unsigned long long CYPublicFunction::GetFileSize(const TString& strPath)
{
#if defined(CYLOGGER_MAC_OS)
    struct stat st;
    if (stat(strPath.c_str(), &st) == 0)
    {
        return st.st_size;
    }
    return 0;
#else
    return std::filesystem::file_size(strPath);
#endif
}

/**
* @brief Get Local Time Zone Offset.
*/
int CYPublicFunction::GetLocalUTCOffsetHours()
{
    std::time_t now = std::time(nullptr);

    std::tm local_tm = *std::localtime(&now);
    std::tm utc_tm = *std::gmtime(&now);

    // Calculate the difference in seconds
    int offset = (local_tm.tm_hour - utc_tm.tm_hour);

    // Processing across days
    if (offset > 12)  offset -= 24;
    if (offset < -12) offset += 24;

    return offset;
}

std::chrono::system_clock::time_point CYPublicFunction::GetLastWriteTime(const TString& strPath)
{
#if defined(CYLOGGER_MAC_OS)
    struct stat st;
    if (stat(strPath.c_str(), &st) == 0)
    {
        auto tp = std::chrono::system_clock::from_time_t(st.st_mtime);

        int offsetHours = GetLocalUTCOffsetHours();
        tp += std::chrono::hours(offsetHours);

        return tp;
    }
    return std::chrono::system_clock::time_point{};
#elif defined(CYLOGGER_ANDROID_OS)
    struct stat st;
    if (stat(strPath.c_str(), &st) == 0)
    {
        auto tp = std::chrono::system_clock::from_time_t(st.st_mtime);

        int offsetHours = GetLocalUTCOffsetHours();
        tp += std::chrono::hours(offsetHours);

        return tp;
    }
    return std::chrono::system_clock::time_point{};
#else
    // Get file creation time
    const auto timepoint = std::filesystem::last_write_time(strPath);
    return  std::chrono::system_clock::now() + (timepoint - std::chrono::file_clock::now());
#endif
}

CYLOGGER_NAMESPACE_END
