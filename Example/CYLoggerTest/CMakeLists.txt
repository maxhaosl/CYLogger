# GUI sample (Windows only)
if(WIN32)
    # Gather resource files
    set(RESOURCES
        CYLoggerTest.rc
        res/CYLoggerTest.ico
    )

    # Create the executable (Win32 subsystem for GUI/MFC apps)
    add_executable(CYLoggerTest WIN32
        CYLoggerTest.cpp
        CYLoggerTestDlg.cpp
        CYLoggerTest.h
        CYLoggerTestDlg.h
        framework.h
        pch.cpp
        pch.h
        targetver.h
        ${RESOURCES}
    )

    # Ensure Visual Studio generates an MFC project linked against the shared runtime
    set_target_properties(CYLoggerTest PROPERTIES
        VS_GLOBAL_Keyword "MFCProj"
        VS_GLOBAL_UseOfMfc "Dynamic"
    )

    # Link CYLogger
    target_link_libraries(CYLoggerTest PRIVATE CYLogger)

    # Define dll import macros when needed
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(CYLoggerTest PRIVATE CYLOGGER_USE_DLL)
    endif()

    # Windows specific link libraries
    target_link_libraries(CYLoggerTest PRIVATE
        comctl32
        gdi32
        user32
        kernel32
        shell32
    )

    # Precompiled header macros
    target_compile_definitions(CYLoggerTest PRIVATE
        WIN32
        _WINDOWS
        _CRT_SECURE_NO_WARNINGS
        _AFXDLL
    )

    # Align runtime outputs with the main project
    set_target_properties(CYLoggerTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )

    # Inherit MSVC runtime library setting from parent project
    if(MSVC)
        if(DEFINED CYLOGGER_MSVC_RUNTIME)
            string(TOUPPER "${CYLOGGER_MSVC_RUNTIME}" _RUNTIME_UPPER)
            if(_RUNTIME_UPPER STREQUAL "MT")
                set(_RUNTIME_VALUE "MultiThreaded")
            elseif(_RUNTIME_UPPER STREQUAL "MTD")
                set(_RUNTIME_VALUE "MultiThreadedDebug")
            elseif(_RUNTIME_UPPER STREQUAL "MDD")
                set(_RUNTIME_VALUE "MultiThreadedDebugDLL")
            else()
                set(_RUNTIME_VALUE "MultiThreadedDLL")
            endif()
            set_target_properties(CYLoggerTest PROPERTIES
                MSVC_RUNTIME_LIBRARY "${_RUNTIME_VALUE}"
            )
        elseif(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
            set_target_properties(CYLoggerTest PROPERTIES
                MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
            )
        endif()
    endif()
endif()