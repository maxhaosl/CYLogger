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
if(BUILD_SHARED_LIBS)
    target_link_libraries(CYLoggerSimpleTest PRIVATE CYLogger)
else()
    target_link_libraries(CYLoggerSimpleTest PRIVATE CYLoggerStatic)
endif()

# Windows specific link libraries
if(WIN32)
    target_link_libraries(CYLoggerSimpleTest PRIVATE
        shell32
        kernel32
    )
endif()

# Configure the runtime output directory
# For multi-config generators, use per-config output directories
if(CMAKE_CONFIGURATION_TYPES)
    set_target_properties(CYLoggerSimpleTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL}"
    )
else()
    set_target_properties(CYLoggerSimpleTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
endif()

# Enable PDB generation for this target on MSVC
if(MSVC)
    # For multi-config generators, set per-configuration PDB output directory
    if(CMAKE_CONFIGURATION_TYPES)
        foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
            string(TOUPPER "${_cfg}" _cfg_upper)
            set_target_properties(CYLoggerSimpleTest PROPERTIES
                PDB_OUTPUT_DIRECTORY_${_cfg_upper} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg_upper}}"
            )
        endforeach()
    else()
        set_target_properties(CYLoggerSimpleTest PROPERTIES
            PDB_NAME "CYLoggerSimpleTest"
            PDB_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    endif()
endif()

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