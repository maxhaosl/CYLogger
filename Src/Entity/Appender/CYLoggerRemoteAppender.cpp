#include "CYCoroutine/CYCoroutine.hpp"
#include "Src/Entity/Appender/CYLoggerRemoteAppender.hpp"
#include "Src/Statistics/CYStatistics.hpp"
#include "Src/Common/Exception/CYExceptionLogFile.hpp"
#include "CYCoroutine/Common/Structure/CYStringUtils.hpp"

#ifdef WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <locale>
#include <codecvt>

CYLOGGER_NAMESPACE_BEGIN

CYLoggerRemoteAppender::CYLoggerRemoteAppender(const TString& strHost) noexcept
    : CYLoggerBufferAppender("RemoteThread")
    , m_bSocketInit(false)
    , m_ipAddr(0)
#ifdef WIN32
    , m_socket(INVALID_SOCKET)
#else
    , m_socket(0)
#endif
{
    EXCEPTION_BEGIN
    {
       m_strHost = CYCOROUTINE_NAMESPACE::CYStringUtils::TString2String(strHost.data());
        size_t index = m_strHost.rfind(TEXT(':'));
        if (index != TString::npos)
        {
            std::string strPort = m_strHost.substr(index + 1);
            m_nPort = atoi(strPort.c_str());
            m_strHost = m_strHost.substr(0, index);
        }
       StartLogThread();
    }
    EXCEPTION_END
}

CYLoggerRemoteAppender::~CYLoggerRemoteAppender() noexcept
{
EXCEPTION_BEGIN
{
    StopLogThread();
    CloseSocket();
#ifdef WIN32
    if (m_bSocketInit)
    {
        WSACleanup();
    }
#endif
}
EXCEPTION_END
}

/**
* @return the unique id for this appender
*/
const ELogType CYLoggerRemoteAppender::GetId() const
{
    return ELogType::LOG_TYPE_REMOTE;
}

/**
* Outputs message on the physical device
* @param msg message to output
* @bFlush if true flushing may be done right after the output. It's for concrete implementation to respect this parameter
*/
void CYLoggerRemoteAppender::Log(const TStringView& strMsg, int nTypeIndex, bool bFlush)
{
    CYFPSCounter::UpdateCounter();
    Statistics()->AddRemoteLine(1);
    Statistics()->AddRemoteBytes(strMsg.size() + TEXT_BYTE_LEN);

    Statistics()->AddRemoteCurrentFPS(CYFPSCounter::GetCurrentFPS());
    Statistics()->AddRemoteAverageFPS(CYFPSCounter::GetAverageFPS());

    WriteSocket(CYCOROUTINE_NAMESPACE::CYStringUtils::TString2String(strMsg.data()));
}

/**
* Get called when this appender become active through attaching to a CYLLoggerImpl object, and it was not active before
*/
void CYLoggerRemoteAppender::OnActivate()
{
    OpenSocket();
}

/**
* Attempt to Flush buffers to a physical device - default implementation does nothing
*/
void CYLoggerRemoteAppender::Flush()
{

}

/**
* Returns the actual size of data on a physical device - default implementation does not respect this and throws an exception
* @returns the actual size of data, located on a physical device
*/
//virtual long GetSize() {
int64_t CYLoggerRemoteAppender::GetSize()
{
    return 0;
}

/**
* Attempt to physically copy data - default implementation does not respect this and throws an exception
*/
void CYLoggerRemoteAppender::Copy(const TString& strTarget)
{

}

/**
* Attempt to physically clear data - default implementation does not respect this and throws an exception
*/
void CYLoggerRemoteAppender::ClearContents()
{

}

/**
 * @brief Get log file name.
*/
const TString& CYLoggerRemoteAppender::GetLogName()
{
    return m_strLogName;
}

/**
 * @brief Force new log file.
*/
void CYLoggerRemoteAppender::ForceNewFile()
{

}

/**
 * @brief Open Socket.
*/
void CYLoggerRemoteAppender::OpenSocket()
{
    IfTrueThrow(m_strHost.empty(), TEXT("Remote appender host cannot empty."));
    IfTrueThrow(m_nPort == 0, TEXT("Remote appender port is zero."));
    if (!m_ipAddr)
    {
        struct hostent* pHostent = gethostbyname(m_strHost.c_str());
#ifdef WIN32
        if (pHostent == nullptr)
        {
            if (WSAGetLastError() == WSANOTINITIALISED)
            {
                WSADATA wsaData;
                int err = WSAStartup(0x101, &wsaData);
                if (err)
                {
                    return;
                }
                pHostent = gethostbyname(m_strHost.c_str());
                m_bSocketInit = true;
            }
            else
            {
                return;
            }
        }
#endif
        if (pHostent == nullptr)
        {
            in_addr_t ip = inet_addr(m_strHost.c_str());
            pHostent = gethostbyaddr((const char*)&ip, sizeof(in_addr_t), AF_INET);
            if (pHostent == nullptr)
            {
                return;
            }
        }
        m_ipAddr = *(in_addr_t*)(pHostent->h_addr);
    }
    // Get a datagram socket.
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if
#ifdef WIN32
    (m_socket == INVALID_SOCKET)
#else
    (m_socket < 0)
#endif
    {
        return;
    }
}

/**
 * @brief Close Socket.
*/
void CYLoggerRemoteAppender::CloseSocket()
{
    if (m_socket)
    {
#ifdef WIN32
        closesocket(m_socket);
#else
        ::close(m_socket);
#endif
        m_socket = 0;
    }
}

/**
 * @brief Write Socket.
*/
void CYLoggerRemoteAppender::WriteSocket(std::string strMsg)
{
    if
#ifdef WIN32
    (m_socket == INVALID_SOCKET)
#else
        (m_socket < 0)
#endif
    {
        return;
    }

    size_t nMsgLen = strMsg.length();
    m_buffer.reserve(nMsgLen);
    memcpy(&m_buffer[0], strMsg.data(), nMsgLen);

    sockaddr_in sain;
    sain.sin_family = AF_INET;
    sain.sin_port = htons(m_nPort);
    sain.sin_addr.s_addr = m_ipAddr;

    while (nMsgLen > 0)
    {
        if (nMsgLen > 900)
        {
            sendto(m_socket, &m_buffer[0], 900, 0, (struct sockaddr*)&sain, sizeof(sain));
            nMsgLen -= 900;
            std::memmove(&m_buffer[0], &m_buffer[900], nMsgLen);
        }
        else
        {
            sendto(m_socket, &m_buffer[0], static_cast<int>(nMsgLen), 0, (struct sockaddr*)&sain, sizeof(sain));
            break;
        }
    }
}

/**
 * @brief ReOpen Socket.
*/
void CYLoggerRemoteAppender::ReOpenSocket()
{
    OpenSocket();
    CloseSocket();
}

CYLOGGER_NAMESPACE_END