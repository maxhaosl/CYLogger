// CYLoggerConsoleTest.cpp : This file contains main(). Program execution starts and ends here.
//

#include <iostream>
#include "ICYLogger.hpp"
#ifdef CYLOGGER_WIN_OS
#include <windows.h>
#include <tchar.h>
#define TCHAR TCHAR
#else
#include <cstring>
#include <unistd.h>
#define MAX_PATH 260
#define GetModuleFileName(A, B, C) (getcwd((char*)(B), (C)))
#define _tcsrchr strchr
#define _T(x) x
#define TCHAR char
#endif

int main()
{
    TCHAR szFilePath[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, szFilePath, sizeof(szFilePath));
    char* lastSlash = _tcsrchr(szFilePath, '/');
    if (lastSlash) lastSlash[1] = 0;

    CY_LOG_CONFIG(szFilePath, LOG_SHOW_CONSOLE_WINDOW, LOG_LAYOUT_TYPE);

    char szBuffer[256] = { 1 };

    int nLineCount = 0;
    for (int i = 0; i < 10; i++)
    {
        CY_LOG_TRACE("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_DEBUG("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_INFO("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_WARN("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ERROR("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_FATAL("%d - %s", nLineCount, "This is a test message!");

        CY_LOG_ESCAPE_TRACE("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ESCAPE_DEBUG("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ESCAPE_INFO("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ESCAPE_WARN("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ESCAPE_ERROR("%d - %s", nLineCount, "This is a test message!");
        CY_LOG_ESCAPE_FATAL("%d - %s", nLineCount, "This is a test message!");

        CY_LOG_DIRECT_TRACE("This is a test message!");
        CY_LOG_HEX_TRACE(szBuffer, sizeof(szBuffer));
    }

    CY_LOG_FREE();

    return 0;
}
