# CYLogger
C++ logging library based on C 20, cross-platform, coroutine support

Log library implemented based on C 20, coroutine support, independent locks, double cache queues, format, range, templates, constraints, etc. Enjoy it.


#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "ICYLogger.hpp"

int main()
{

    TCHAR szFilePath[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, szFilePath, sizeof(szFilePath));
    (_tcsrchr(szFilePath, _T('\\')))[1] = 0;

    CY_LOG_CONFIG(szFilePath, LOG_SHOW_CONSOLE_WINDOW, LOG_LAYOUT_TYPE);


    char szBuffer[256] = { 1 };

    int nLineCount = 0;
    for (int i = 0; i < 10; i++)
    {
        CY_LOG_TRACE(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_DEBUG(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_INFO(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_WARN(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ERROR(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_FATAL(_T("%d - %s"), nLineCount, _T("This is a test message!"));

        CY_LOG_ESCAPE_TRACE(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_DEBUG(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_INFO(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_WARN(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_ERROR(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_FATAL(_T("%d - %s"), nLineCount, _T("This is a test message!"));

        CY_LOG_DIRECT_TRACE(_T("This is a test message!"));
        CY_LOG_HEX_TRACE(szBuffer, sizeof(szBuffer));
    }

    CY_LOG_FREE();

    return 0;
}

