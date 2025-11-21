#include "Entity/Appender/CYLoggerConsoleAppender.hpp"
#include "Entity/Appender/CYLoggerAppenderDefine.hpp"
#include "Statistics/CYStatistics.hpp"

CYLOGGER_NAMESPACE_BEGIN

CYLoggerConsoleAppender::CYLoggerConsoleAppender(const TStringView& strConsoleTile, bool bWindow)
    : CYLoggerBufferAppender("ConsoleThread")
    , m_bWindow(bWindow)
    , m_strTitle(strConsoleTile)
    , m_bForceNewFile(false)
{
    if (m_bWindow)
    {
#ifdef CYLOGGER_WIN_OS
        AllocConsole();
        SetConsoleTitle(m_strTitle.data());
#endif
    }

    StartLogThread();
}

CYLoggerConsoleAppender::~CYLoggerConsoleAppender()
{
    StopLogThread();

    if (m_bWindow)
    {
#ifdef CYLOGGER_WIN_OS
        FreeConsole();
#endif
    }
}

/**
* @return the unique id for this appender
*/
const ELogType CYLoggerConsoleAppender::GetId() const
{
    return ELogType::LOG_TYPE_NONE;
}

/**
* Outputs message on the physical device
* @param msg message to output
* @bFlush if true flushing may be done right after the output. It's for concrete implementation to respect this parameter
*/
void CYLoggerConsoleAppender::Log(const TStringView& strMsg, int nTypeIndex, bool bFlush)
{
    CYFPSCounter::UpdateCounter();

    ClearConsole();
#ifdef CYLOGGER_WIN_OS
    if (m_bWindow)
    {
        DWORD color = TWHITE;
        switch (strMsg[nTypeIndex])
        {
        case 'I': color = TWHITE;	break;
        case 'D': color = TGREEN;	break;
        case 'T': color = TBLUE;	break;
        case 'W': color = TYELLOW;	break;
        case 'E': color = TRED;		break;
        case 'F': color = TRED;		break;
        default:  color = TWHITE;	break;
        }

        unsigned long dwWrittenSize = 0;
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), strMsg.data(), (DWORD)cy_strlen(strMsg.data()), &dwWrittenSize, nullptr);
        WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), TEXT("\r\n"), (DWORD)cy_strlen(TEXT("\r\n")), &dwWrittenSize, nullptr);
    }
    else
    {
        OutputDebugString(strMsg.data());
        OutputDebugString(TEXT("\r\n"));
    }
#else
    printf("%s", strMsg.data());
    printf("\n");
#endif

    Statistics()->AddConsoleLine(1);
    Statistics()->AddConsoleBytes(strMsg.size() + TEXT_BYTE_LEN + TEXT_BYTE_LEN);

    Statistics()->AddConsoleCurrentFPS(CYFPSCounter::GetCurrentFPS());
    Statistics()->AddConsoleAverageFPS(CYFPSCounter::GetAverageFPS());
}

/**
* Get called when this appender become active through attaching to a CYLLoggerImpl object, and it was not active before
*/
void CYLoggerConsoleAppender::OnActivate()
{
    return;
}

/**
* Attempt to Flush buffers to a physical device
*/
void CYLoggerConsoleAppender::Flush()
{
    return;
}

/**
* Returns the actual size of data on a physical device
*/
//virtual long GetSize() {
int64_t CYLoggerConsoleAppender::GetSize()
{
    return 0;
}

/**
* Attempt to physically copy data
*/
void CYLoggerConsoleAppender::Copy(const TString& strTarget)
{
    return;
}

/**
* Attempt to physically clear data
*/
void CYLoggerConsoleAppender::ClearContents()
{
    return;
}

/**
 * @brief Get log file name.
*/
const TString& CYLoggerConsoleAppender::GetLogName()
{
    return m_strLogName;
}

/**
 * @brief Force new log file.
*/
void CYLoggerConsoleAppender::ForceNewFile()
{
    m_bForceNewFile = true;
}

/**
 * @brief Clear console.
*/
void CYLoggerConsoleAppender::ClearConsole()
{
    if (!m_bForceNewFile)
        return;

    m_bForceNewFile = false;

#ifdef CYLOGGER_WIN_OS
    if (m_bWindow)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // Get console information, used to get the size of the console
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
        GetConsoleScreenBufferInfo(hConsole, &bufferInfo);

        COORD cdBufferSize = bufferInfo.dwSize;
        DWORD dwConsoleSize = cdBufferSize.X * cdBufferSize.Y;

        // Get the current console cursor position
        COORD cdCursorPosition = { 0, 0 };
        SetConsoleCursorPosition(hConsole, cdCursorPosition);

        // Fill the console buffer and clear the screen contents
        FillConsoleOutputCharacter(hConsole, TEXT(' '), dwConsoleSize, cdCursorPosition, nullptr);
        FillConsoleOutputAttribute(hConsole, bufferInfo.wAttributes, dwConsoleSize, cdCursorPosition, nullptr);
    }
    else
    {
        system("cls");
    }

#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

CYLOGGER_NAMESPACE_END
