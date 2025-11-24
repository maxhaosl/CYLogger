#include "Entity/Schedule/CYLoggerClearLogFile.hpp"
#include "Config/CYLoggerConfig.hpp"
#include "Common/CYPathConvert.hpp"
#include "Entity/Appender/CYLoggerBaseAppender.hpp"
#include "Entity/CYLoggerEntity.hpp"
#include "Entity/CYLoggerEntityFactory.hpp"
#include "Common/Exception/CYExceptionLogFile.hpp"
#include "Common/CYPublicFunction.hpp"

#include <filesystem>
#include <chrono>
#include <ctime>
#ifdef CYLOGGER_WIN_OS
#include <windows.h>
#else
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#endif

CYLOGGER_NAMESPACE_BEGIN

CYLoggerClearLogFile::CYLoggerClearLogFile()
    : m_bFirstProcess(true)
{
}

CYLoggerClearLogFile::~CYLoggerClearLogFile()
{
}

/**
 * @brief Set restriction rules
*/
void CYLoggerClearLogFile::SetRestriction(bool m_bClearUnLogFile, int nLimitTimeClearLog, int nLimitTimeExpiredFile, int nCheckFileSizeTime, int nCheckFileCountTime, int nCheckFileSize, int nFileCountPerType, int nCheckFileTypeSize, int nCheckALLFileSize)
{
    m_bEnableClearUnLogFile = m_bClearUnLogFile;
    m_nLimitTimeClearLog = nLimitTimeClearLog;
    m_nLimitTimeExpiredFile = nLimitTimeExpiredFile;
    m_nCheckFileSizeTime = nCheckFileSizeTime;
    m_nCheckFileCountTime = nCheckFileCountTime;
    m_nCheckFileSize = nCheckFileSize;
    m_nFileCountPerType = nFileCountPerType;
    m_nCheckFileTypeSize = nCheckFileTypeSize;
    m_nCheckALLFileSize = nCheckALLFileSize;
}

/**
 * @brief Clean up log processing.
*/
void CYLoggerClearLogFile::ProcessClearLog(const std::list<ELogType>& lstLogType)
{
    std::list<CYLogFileInfo> lstUsedLogFile;                                // Current Log File.
    std::list<TString>       lstLogDirFile;                                 // All log files in the log directory.
    std::list<TString>       lstNotLogDirFile;                              // All non-log files in the log directory.
    FileClassMap             mapFileClass;

    ProcessRunningLogFile(lstLogType, lstUsedLogFile);                      // Get running log file information.
    ProcessPublicLogInfo(lstLogType, lstLogDirFile, lstNotLogDirFile);      // Get log file information.
    PreprocessData(lstUsedLogFile, lstLogDirFile, mapFileClass);

    if (m_bFirstProcess)
    {
        ProcessClearNonLog(lstNotLogDirFile);                                   // Clean out all non-log files.
    }

    ProcessClearLogCount(mapFileClass);                                     // Clean the number of log files.
    ProcessClearLogExpired(mapFileClass);                                   // Clean up expired log files.

    if (m_bFirstProcess || m_objElapsedCheckSizeTime.Elapsed() > m_nCheckFileSizeTime)
    {
        ProcessClearLogTypeSize(mapFileClass);                              // Log files of each type that exceed the specified size are cleaned.
        ProcessClearLogALLSize(mapFileClass);                               // Log files that exceed the specified size are cleaned.
        m_objElapsedCheckSizeTime.Reset();
    }

    m_bFirstProcess = false;
}

/**
 * @brief Get running log file information.
*/
void CYLoggerClearLogFile::ProcessRunningLogFile(const std::list<ELogType>& lstLogType, std::list<CYLogFileInfo>& lstUsedLogFile)
{
    lstUsedLogFile.clear();
    auto ptrFactory = LoggerEntityFactory();
    if (!ptrFactory)
        return;

    for (auto& eLogType : lstLogType)
    {
        SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> ptrEntity = ptrFactory->GetLoggerEntity(eLogType);
        if (!ptrEntity)
            continue;

        const TString strLogName = ptrEntity->GetLogName();
        if (strLogName.empty())
            continue;

        CYLogFileInfo objFileInfo;
        objFileInfo.eLogType = ptrEntity->GetId();
        objFileInfo.strLogFilePath = strLogName;
        objFileInfo.strCheckName = GetCheckName(strLogName);
        lstUsedLogFile.emplace_back(objFileInfo);
    }
}

/**
 * @brief Get log file information.
*/
void CYLoggerClearLogFile::ProcessPublicLogInfo(const std::list<ELogType>& lstLogType, std::list<TString>& lstLogFile, std::list<TString>& lstNotLogFile)
{
    // Get log path.
    TString strLogPath = CYPathConvert::ConvertFilePath(TEXT(""), LoggerConfig()->GetLogPath().c_str(), LOG_DIR);

    // Enum log file.
    EnumLogFile(strLogPath, lstLogFile);

    // Enum non-log file.
    EnumNotLogFile(strLogPath, lstNotLogFile);
}

/**
 * @brief Preprocess some data.
*/
void CYLoggerClearLogFile::PreprocessData(const std::list<CYLogFileInfo>& lstUsedLogFile, const std::list<TString>& lstLogFile, FileClassMap& mapLogFileInfo)
{
    for (auto& strLogFile : lstLogFile)
    {
        bool bUsed = IsUsedLogFile(lstUsedLogFile, strLogFile);
        if (bUsed) continue; // Filter out log files in use

        // filter Exception.log.
        if (cy_strstr(strLogFile.c_str(), LoggerConfig()->GetErrorLogName().c_str()) != nullptr)
            continue;

        CYLogFileInfo objFileInfo;
        objFileInfo.strLogFilePath = strLogFile;
        objFileInfo.strCheckName = GetCheckName(strLogFile);
        objFileInfo.eLogType = GetLogFileType(lstUsedLogFile, objFileInfo.strCheckName);

        GetFileInfomation(objFileInfo);

        TString strFileName = CYPublicFunction::GetFileName(strLogFile);

        std::map<TString, CYLogFileInfo> mapTypeLogFile;
        auto iterFind = mapLogFileInfo.find(objFileInfo.eLogType);
        if (iterFind != mapLogFileInfo.end())
        {
            mapTypeLogFile = iterFind->second;
        }

        mapTypeLogFile[strFileName] = objFileInfo;
        mapLogFileInfo[objFileInfo.eLogType] = mapTypeLogFile;
    }
}

/**
 * @brief Clean out all non-log files
*/
void CYLoggerClearLogFile::ProcessClearNonLog(const std::list<TString>& lstNotLogFile)
{
    for (auto& objFileInfo : lstNotLogFile)
    {
        CYPublicFunction::Remove(objFileInfo.c_str());
    }
}

/**
 * @brief Clean the number of log files.
*/
void CYLoggerClearLogFile::ProcessClearLogCount(FileClassMap& mapLogFileInfo)
{
    FileClassMap mapLogFileInfo2 = mapLogFileInfo;
    for (const auto& mapLog : mapLogFileInfo2)
    {
        // List of keys to delete
        std::vector<TString> lstKeysToDelete;

        // Log file map for each type.
        std::map<TString, CYLogFileInfo> mapTypeFileInfo = mapLog.second;
        if (mapTypeFileInfo.size() > m_nFileCountPerType)
        {
            unsigned nSize = static_cast<unsigned>(mapTypeFileInfo.size());
            for (auto& objFileInfo : mapTypeFileInfo)
            {
                if (nSize <= static_cast<unsigned>(m_nFileCountPerType))
                    break;

                lstKeysToDelete.push_back(objFileInfo.first);
                CYPublicFunction::Remove(objFileInfo.second.strLogFilePath);
                nSize--;
            }

            // Delete key.
            for (auto& strKey : lstKeysToDelete)
            {
                mapTypeFileInfo.erase(strKey);
            }

            mapLogFileInfo[mapLog.first] = mapTypeFileInfo;
        }
    }
}

/**
 * @brief Clean up expired log files.
*/
void CYLoggerClearLogFile::ProcessClearLogExpired(FileClassMap& mapLogFileInfo)
{
    FileClassMap mapLogFileInfo2 = mapLogFileInfo;
    for (const auto& mapLog : mapLogFileInfo2)
    {
        // List of keys to delete
        std::vector<TString> lstKeysToDelete;

        // Log file map for each type.
        std::map<TString, CYLogFileInfo> mapTypeFileInfo = mapLog.second;
        for (auto& objFileInfo : mapTypeFileInfo)
        {
            auto tpEndTime = std::chrono::system_clock::now();
            auto objDuration = std::chrono::duration_cast<std::chrono::hours>(tpEndTime - objFileInfo.second.tpCreateTime);
            int nHours = objDuration.count();
            if (nHours > m_nLimitTimeExpiredFile)
            {
                lstKeysToDelete.push_back(objFileInfo.first);
                CYPublicFunction::Remove(objFileInfo.second.strLogFilePath);
            }
        }

        // Delete key.
        for (auto& strKey : lstKeysToDelete)
        {
            mapTypeFileInfo.erase(strKey);
        }

        mapLogFileInfo[mapLog.first] = mapTypeFileInfo;
    }
}

/**
 * @brief Log files of each type that exceed the specified size are cleaned.
*/
void CYLoggerClearLogFile::ProcessClearLogTypeSize(FileClassMap& mapLogFileInfo)
{
    FileClassMap mapLogFileInfo2 = mapLogFileInfo;
    for (const auto& mapLog : mapLogFileInfo2)
    {
        uint64_t nLogTotalBytes = 0;
        std::map<TString, CYLogFileInfo> mapTypeFileInfo = mapLog.second;
        for (auto& objFileInfo : mapTypeFileInfo)
        {
            nLogTotalBytes += objFileInfo.second.nSize;
        }

        if (nLogTotalBytes < m_nCheckFileTypeSize)
            continue;

        // List of keys to delete
        std::vector<TString> lstKeysToDelete;

        // Log file map for each type.
        for (auto& objFileInfo : mapTypeFileInfo)
        {
            if (nLogTotalBytes < m_nCheckFileTypeSize)
                break;

            nLogTotalBytes -= objFileInfo.second.nSize;

            lstKeysToDelete.push_back(objFileInfo.first);
            CYPublicFunction::Remove(objFileInfo.second.strLogFilePath);
        }

        // Delete key.
        for (auto& strKey : lstKeysToDelete)
        {
            mapTypeFileInfo.erase(strKey);
        }

        mapLogFileInfo[mapLog.first] = mapTypeFileInfo;
    }
}

/**
 * @brief Log files that exceed the specified size are cleaned.
*/
void CYLoggerClearLogFile::ProcessClearLogALLSize(FileClassMap& mapLogFileInfo)
{
    std::multimap<TString, CYLogFileInfo> mapALLFile;

    uint64_t nALLSize = 0;
    for (const auto& mapLog : mapLogFileInfo)
    {
        for (const auto& fileInfo : mapLog.second)
        {
            nALLSize += fileInfo.second.nSize;
            mapALLFile.insert(std::pair<TString, CYLogFileInfo>(fileInfo.first, fileInfo.second));
        }
    }

    if (nALLSize < m_nCheckALLFileSize) return;

    for (auto& objInfo : mapALLFile)
    {
        if (nALLSize < m_nCheckALLFileSize) break;

        nALLSize -= objInfo.second.nSize;
        CYPublicFunction::Remove(objInfo.second.strLogFilePath);
    }
}

/**
 * @brief Enumerate files in the specified directory.
*/
void CYLoggerClearLogFile::EnumLogFile(TString strLogPath, std::list<TString>& vecLogList)
{
#ifdef CYLOGGER_WIN_OS
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA struFindFileData;
    hFindFile = FindFirstFile((strLogPath + TEXT("*")).c_str(), &struFindFileData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if (!cy_tcscmp(struFindFileData.cFileName, TEXT(".")) || !cy_tcscmp(struFindFileData.cFileName, TEXT("..")))
        {
            continue;
        }

        if (0 != (struFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            EnumLogFile(strLogPath + LOG_SEPARATOR + struFindFileData.cFileName + LOG_SEPARATOR, vecLogList);
        }
        else
        {
            if (cy_strstr(struFindFileData.cFileName, TEXT(".log")))
            {
                vecLogList.push_back(strLogPath + struFindFileData.cFileName);
            }
        }
    } while (FindNextFile(hFindFile, &struFindFileData));

#else
    DIR* dir = opendir(strLogPath.c_str());
    if (!dir)
    {
        return;
    }
    dirent* p = nullptr;
    while ((p = readdir(dir)) != nullptr)
    {
        // Skip "." and ".." directories
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
        {
            continue;
        }

        TString strFullPath = strLogPath + p->d_name;
        struct stat s;
        if (stat(strFullPath.c_str(), &s) == 0)
        {
            if (S_ISDIR(s.st_mode))
            {
                EnumLogFile(strFullPath + "/", vecLogList);
            }
            else
            {
                // Only add .log files
                std::string strName = p->d_name;
                if (strName.find(".log") != std::string::npos)
                {
                    vecLogList.push_back(strLogPath + p->d_name);
                }
            }
        }
    }
    closedir(dir);
#endif
}

/**
 * @brief Enumerate files in the specified directory.
*/
void CYLoggerClearLogFile::EnumNotLogFile(TString strLogPath, std::list<TString>& vecNotLogList)
{
#ifdef CYLOGGER_WIN_OS
    HANDLE hFindFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA struFindFileData;
    hFindFile = FindFirstFile((strLogPath + TEXT("*")).c_str(), &struFindFileData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return;
    }
    do
    {
        if (!cy_tcscmp(struFindFileData.cFileName, TEXT(".")) || !cy_tcscmp(struFindFileData.cFileName, TEXT("..")))
        {
            continue;
        }

        if (0 != (struFindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            EnumNotLogFile(strLogPath + LOG_SEPARATOR + struFindFileData.cFileName + LOG_SEPARATOR, vecNotLogList);
        }
        else
        {
            if (!cy_strstr(struFindFileData.cFileName, TEXT(".log")))
            {
                vecNotLogList.push_back(strLogPath + struFindFileData.cFileName);
            }
        }
    } while (FindNextFile(hFindFile, &struFindFileData));
#else
    DIR* dir = opendir(strLogPath.c_str());
    if (!dir)
    {
        return;
    }
    dirent* p = nullptr;
    while ((p = readdir(dir)) != nullptr)
    {
        // Skip "." and ".." directories
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
        {
            continue;
        }

        TString strFullPath = strLogPath + p->d_name;
        struct stat s;
        if (stat(strFullPath.c_str(), &s) == 0)
        {
            if (S_ISDIR(s.st_mode))
            {
                EnumNotLogFile(strFullPath + "/", vecNotLogList);
            }
            else
            {
                std::string strName = p->d_name;
                if (strName.find(".log") == std::string::npos)
                {
                    vecNotLogList.push_back(strLogPath + p->d_name);
                }
            }
        }
    }
    closedir(dir);
#endif
}

/**
 * @brief Get log file information.
*/
void CYLoggerClearLogFile::GetFileInfomation(CYLogFileInfo& objLogFileInfo)
{
    EXCEPTION_BEGIN
    {
        // Get file size
        objLogFileInfo.nSize = CYPublicFunction::GetFileSize(objLogFileInfo.strLogFilePath);

    // Get create time
    objLogFileInfo.tpCreateTime = CYPublicFunction::GetLastWriteTime(objLogFileInfo.strLogFilePath);
    }
        EXCEPTION_END
}

/**
 * @brief Get Check Name.
*/
TString CYLoggerClearLogFile::GetCheckName(const TString& strFileName)
{
    TString strCheckName = CYPublicFunction::GetFileName(strFileName);
    size_t index = TString::npos;

    index = strCheckName.find_last_of(TEXT("_"));
    if (index != TString::npos)
    {
        TString strLeft = strCheckName.substr(0, index);
        strCheckName = strLeft;
    }
    else
    {
        index = strCheckName.find_last_of(TEXT("."));
        if (index != TString::npos)
        {
            TString strLeft = strCheckName.substr(0, index);
            strCheckName = strLeft;
        }
    }
    return strCheckName;
}

template<typename _Tp>
bool upperEqual(_Tp a, _Tp b)
{
    return ((islower(a) ? a - 32 : a) == (islower(b) ? b - 32 : b)) ? true : false;
}

/**
 * @brief Get Log file type.
*/
ELogType CYLoggerClearLogFile::GetLogFileType(const std::list<CYLogFileInfo>& lstLogFile, const TString& strCheckName)
{
    ELogType eLogType = ELogType::LOG_TYPE_NONE;
    for (const auto& objFileInfo : lstLogFile)
    {
        if (strCheckName.compare(objFileInfo.strCheckName) == 0)
            return objFileInfo.eLogType;;
    }
    return eLogType;
}

/**
 * @brief Determine whether the file is used.
*/
bool CYLoggerClearLogFile::IsUsedLogFile(const std::list<CYLogFileInfo>& lstUsedLogFile, const TString& strFileName)
{
    for (const auto& objFileInfo : lstUsedLogFile)
    {
        if (objFileInfo.strLogFilePath.compare(strFileName) == 0)
        {
            return true;
        }
    }
    return false;
}

CYLOGGER_NAMESPACE_END
