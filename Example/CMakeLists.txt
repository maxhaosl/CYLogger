# Console examples
add_subdirectory(CYLoggerConsoleTest)

# Lite example without CYCoroutine
add_subdirectory(CYLoggerSimpleTest)

# Only build the Win32 GUI sample on Windows hosts
# Note: CYLoggerTest uses MFC DLL which requires MD/MDD runtime (dynamic runtime), skip it for MT/MTD builds
if(WIN32)
    if(NOT DEFINED CYLOGGER_MSVC_RUNTIME)
        # Default to MD if not specified
        add_subdirectory(CYLoggerTest)
    else()
        string(TOUPPER "${CYLOGGER_MSVC_RUNTIME}" _RUNTIME_UPPER)
        if(_RUNTIME_UPPER STREQUAL "MD" OR _RUNTIME_UPPER STREQUAL "MDD")
            # MD and MDD are dynamic runtimes, compatible with MFC
            add_subdirectory(CYLoggerTest)
        else()
            message(STATUS "Skipping CYLoggerTest (MFC GUI) - requires MD/MDD runtime (dynamic), but CYLOGGER_MSVC_RUNTIME=${CYLOGGER_MSVC_RUNTIME}")
        endif()
    endif()
endif()