# Minimal example without CYCoroutine dependency

# Collect sources
set(SIMPLE_TEST_SOURCES
    CYLoggerSimpleTest.cpp
)

# Create the executable
add_executable(CYLoggerSimpleTest ${SIMPLE_TEST_SOURCES})

# Add include paths
target_include_directories(CYLoggerSimpleTest PRIVATE 
    ${CMAKE_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Src
)

# Link against CYLogger
target_link_libraries(CYLoggerSimpleTest PRIVATE CYLogger)

# Windows specific link libraries
if(WIN32)
    target_link_libraries(CYLoggerSimpleTest PRIVATE
        shell32
        kernel32
    )
endif()

# Configure the runtime output directory
set_target_properties(CYLoggerSimpleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Bin/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE}
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
        set_target_properties(CYLoggerSimpleTest PROPERTIES
            MSVC_RUNTIME_LIBRARY "${_RUNTIME_VALUE}"
        )
    elseif(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set_target_properties(CYLoggerSimpleTest PROPERTIES
            MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}"
        )
    endif()
endif()