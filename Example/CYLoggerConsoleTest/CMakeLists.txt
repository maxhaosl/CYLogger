# Console test target
add_executable(CYLoggerConsoleTest CYLoggerConsoleTest.cpp)

# Link against the main CYLogger target
target_link_libraries(CYLoggerConsoleTest PRIVATE CYLogger)

# Define import macros when linking against the DLL
if(BUILD_SHARED_LIBS)
    target_compile_definitions(CYLoggerConsoleTest PRIVATE CYLOGGER_USE_DLL)
endif()

# Platform specific tweaks
if(WIN32)
    target_link_libraries(CYLoggerConsoleTest PRIVATE
        shell32
        kernel32
    )
endif()

# Place the binary alongside the core outputs
set_target_properties(CYLoggerConsoleTest PROPERTIES
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
        set_target_properties(CYLoggerConsoleTest PROPERTIES
            MSVC_RUNTIME_LIBRARY "${_RUNTIME_VALUE}"
        )
    elseif(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set_target_properties(CYLoggerConsoleTest PROPERTIES
            MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
        )
    endif()
endif()