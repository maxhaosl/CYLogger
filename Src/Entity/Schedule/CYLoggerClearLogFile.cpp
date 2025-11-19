#include "Src/Entity/Schedule/CYLoggerClearLogFile.hpp"
#include "Src/Config/CYLoggerConfig.hpp"
#include "Src/Common/CYPathConvert.hpp"
#include "Src/Entity/Appender/CYLoggerBaseAppender.hpp"
#include "Src/Entity/CYLoggerEntity.hpp"
#include "Src/Entity/CYLoggerEntityFactory.hpp"
#include "Src/Common/Exception/CYExceptionLogFile.hpp"
#include "Src/Common/CYPublicFunction.hpp"

#include <filesystem>
#include <chrono>
#include <ctime>

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
    for (auto& eLogType : lstLogType)
    {
        SharePtr<CYLoggerEntity<CYLoggerBaseAppender>> ptrEntity = LoggerEntityFactory()->GetLoggerEntity(eLogType);
        if (ptrEntity->GetLogName().empty()) 
            continue;

        CYLogFileInfo objFileInfo;
        objFileInfo.eLogType = ptrEntity->GetId();
        objFileInfo.strLogFilePath = ptrEntity->GetLogName();
        objFileInfo.strCheckName = GetCheckName(ptrEntity->GetLogName());
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
        std::filesystem::remove(objFileInfo.c_str());
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
                std::filesystem::remove(objFileInfo.second.strLogFilePath);
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
                std::filesystem::remove(objFileInfo.second.strLogFilePath);
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
            std::filesystem::remove(objFileInfo.second.strLogFilePath);
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
        std::filesystem::remove(objInfo.second.strLogFilePath);
    }
}

/**
 * @brief Enumerate files in the specified directory.
*/
void CYLoggerClearLogFile::EnumLogFile(TString strLogPath, std::list<TString>& vecLogList)
{
#ifdef WIN32
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
        if (p->d_name[0] != '.')
        {
            TString strFullPath = strLogPath + p->d_name;
            struct stat s;
            stat(strFullPath.c_str(), &s);
            if (S_ISDIR(s.st_mode))
            {
                EnumLogFile(strFullPath + "/", vecLogList);
            }
            else
            {
                vecLogList.push_back(strLogPath + p->d_name);
            }
        }
    }
    closedir(dir);//关闭指定目录
#endif
}

/**
 * @brief Enumerate files in the specified directory.
*/
void CYLoggerClearLogFile::EnumNotLogFile(TString strLogPath, std::list<TString>& vecNotLogList)
{
#ifdef WIN32
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
        if (p->d_name[0] != '.')
        {
            TString strFullPath = strLogPath + p->d_name;
            struct stat s;
            stat(strFullPath.c_str(), &s);
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
    closedir(dir);//关闭指定目录
#endif
    }

/**
 * @brief Get log file information.
*/
void CYLoggerClearLogFile::GetFileInfomation(CYLogFileInfo& objLogFileInfo)
{
    EXCEPTION_BEGIN

    // Get file size
    objLogFileInfo.nSize = std::filesystem::file_size(objLogFileInfo.strLogFilePath);

    // Get file creation time
    const auto timepoint = std::filesystem::last_write_time(objLogFileInfo.strLogFilePath);
    objLogFileInfo.tpCreateTime = std::chrono::clock_cast<std::chrono::system_clock>(timepoint);

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
        if(strCheckName.compare(objFileInfo.strCheckName) == 0)
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