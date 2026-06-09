cmake_minimum_required(VERSION 3.16)
project(CYLogger VERSION 1.0.0 LANGUAGES CXX)

# Store the project root directory for use by subprojects
set(CYLOGGER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Save BUILD_SHARED_LIBS after project() — it may be reset by subdirectory project() calls.
set(_CYLOGGER_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# Detect iOS from CMAKE_SYSTEM_NAME (set by ios.toolchain.cmake) or CMAKE_OSX_SYSROOT
if(NOT DEFINED IOS OR NOT IOS)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(IOS ON CACHE BOOL "Building for iOS" FORCE)
    elseif(CMAKE_OSX_SYSROOT MATCHES "iPhone|iOS")
        set(IOS ON CACHE BOOL "Building for iOS" FORCE)
    endif()
endif()

# For shared libraries, Debug builds on non-Windows use CYLoggerD output name.
# For static libraries, Debug builds use CYLoggerD; Release uses CYLogger (no suffix) on Android/macOS.
set(_CYLOGGER_SHARED_OUTPUT_NAME "CYLogger")
set(_CYLOGGER_STATIC_OUTPUT_NAME "CYLoggerStatic")

# Configure the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always force the compiler to use C++20
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
    # /FS - Force synchronous PDB writes (required when multiple cl.exe write to same PDB)
    # Apply to per-configuration flags so MSBuild generates the flag in .vcxproj files
    # (generator expression $<$<COMPILE_LANGUAGE:CXX>:/FS> does not reliably work here)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /FS")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /FS")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()

# Force PDB generation for all configurations including Release
# MSVC Release builds skip PDB generation by default, but we want them available
if(MSVC)
    # Preserve the runtime selected by CMAKE_MSVC_RUNTIME_LIBRARY/CYLOGGER_MSVC_RUNTIME.
    # Only append debug-info and optimization flags here instead of hardcoding /MD or /MDd.
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /Zi /Od")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /Zi /O2")
    string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " /Zi /O1")
    string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " /Zi /O2")

    # Linker flags for PDB generation
    string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " /DEBUG:FULL")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE " /DEBUG:FULL /PDBALTPATH:%_PDB%")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_DEBUG " /DEBUG:FULL")
    string(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE " /DEBUG:FULL /PDBALTPATH:%_PDB%")
endif()

if(MSVC)
    set(_CYLOGGER_VALID_RUNTIMES "MD" "MT" "MDD" "MTD")
    set(CYLOGGER_MSVC_RUNTIME "MD" CACHE STRING "Choose the MSVC runtime library to link against (/MD, /MT, /MDD, /MTD).")
    set_property(CACHE CYLOGGER_MSVC_RUNTIME PROPERTY STRINGS "MD" "MT" "MDD" "MTD")

    string(TOUPPER "${CYLOGGER_MSVC_RUNTIME}" _CYLOGGER_RUNTIME_UPPER)
    list(FIND _CYLOGGER_VALID_RUNTIMES "${_CYLOGGER_RUNTIME_UPPER}" _CYLOGGER_RUNTIME_INDEX)
    if(_CYLOGGER_RUNTIME_INDEX EQUAL -1)
        message(FATAL_ERROR
            "CYLOGGER_MSVC_RUNTIME must be one of MT, MD, MTD, or MDD (got ${CYLOGGER_MSVC_RUNTIME}).")
    endif()

    if(_CYLOGGER_RUNTIME_UPPER STREQUAL "MT")
        set(_CYLOGGER_MSVC_RUNTIME_VALUE "MultiThreaded")
    elseif(_CYLOGGER_RUNTIME_UPPER STREQUAL "MTD")
        set(_CYLOGGER_MSVC_RUNTIME_VALUE "MultiThreadedDebug")
    elseif(_CYLOGGER_RUNTIME_UPPER STREQUAL "MDD")
        set(_CYLOGGER_MSVC_RUNTIME_VALUE "MultiThreadedDebugDLL")
    else()
        set(_CYLOGGER_MSVC_RUNTIME_VALUE "MultiThreadedDLL")
    endif()

    set(CMAKE_MSVC_RUNTIME_LIBRARY "${_CYLOGGER_MSVC_RUNTIME_VALUE}" CACHE STRING "MSVC runtime selection" FORCE)
    message(STATUS "CYLOGGER_MSVC_RUNTIME=${CYLOGGER_MSVC_RUNTIME}, CMAKE_MSVC_RUNTIME_LIBRARY=${_CYLOGGER_MSVC_RUNTIME_VALUE}")
    set(CYLOGGER_RUNTIME_SUFFIX "${CYLOGGER_MSVC_RUNTIME}")
else()
    set(CYLOGGER_RUNTIME_SUFFIX "")
endif()

# Default build type (only used for single-config generators)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# CPU architecture list setup (must be before platform detection)
set(_CYLOGGER_OSX_ARCH_LIST "")
set(_CYLOGGER_OSX_ARCH_DEFINED FALSE)
if(DEFINED CMAKE_OSX_ARCHITECTURES)
    string(STRIP "${CMAKE_OSX_ARCHITECTURES}" _CYLOGGER_OSX_ARCH_STRIPPED)
    if(NOT "${_CYLOGGER_OSX_ARCH_STRIPPED}" STREQUAL "")
        set(_CYLOGGER_OSX_ARCH_DEFINED TRUE)
        set(_CYLOGGER_OSX_ARCH_LIST ${CMAKE_OSX_ARCHITECTURES})
        list(JOIN _CYLOGGER_OSX_ARCH_LIST ";" _CYLOGGER_OSX_ARCH_JOINED)
    endif()
endif()

# Platform detection
if(WIN32)
    set(CYLOGGER_PLATFORM "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(APPLE)
    if(IOS)
        set(CYLOGGER_PLATFORM "iOS")
        if(NOT DEFINED CYLOGGER_ARCH)
            # TARGET_ARCH distinguishes arm64 device from arm64-simulator (both -> arm64 for compiler).
            # Use TARGET_ARCH for the output directory subdir name, but NOT for -arch flags.
            if(DEFINED TARGET_ARCH AND NOT "${TARGET_ARCH}" STREQUAL "")
                set(CYLOGGER_ARCH "${TARGET_ARCH}" CACHE STRING "Output subdirectory name (may differ from compiler arch)")
            elseif(_CYLOGGER_OSX_ARCH_DEFINED)
                set(CYLOGGER_ARCH "${_CYLOGGER_OSX_ARCH_JOINED}" CACHE STRING "Output subdirectory name (may differ from compiler arch)")
            else()
                set(CYLOGGER_ARCH "arm64" CACHE STRING "Output subdirectory name (may differ from compiler arch)")
            endif()
        endif()
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
        endif()
    else()
        set(CYLOGGER_PLATFORM "MacOS")
        # Minimum macOS deployment target
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        endif()
    endif()
elseif(ANDROID)
    set(CYLOGGER_PLATFORM "Android")
    add_definitions(-DCYLOGGER_ANDROID_OS)
    add_definitions(-DCYLOGGER_LINUX_OS)
elseif(UNIX)
    set(CYLOGGER_PLATFORM "Linux")
endif()

# CPU architecture detection
# Use a custom variable instead of relying solely on CMAKE_SYSTEM_PROCESSOR
# Normalize CMAKE_OSX_ARCHITECTURES because CMake defines it as an empty string by default
if(DEFINED CMAKE_OSX_ARCHITECTURES)
    string(STRIP "${CMAKE_OSX_ARCHITECTURES}" _CYLOGGER_OSX_ARCH_STRIPPED)
    if(NOT "${_CYLOGGER_OSX_ARCH_STRIPPED}" STREQUAL "")
        set(_CYLOGGER_OSX_ARCH_DEFINED TRUE)
        set(_CYLOGGER_OSX_ARCH_LIST ${CMAKE_OSX_ARCHITECTURES})
        list(JOIN _CYLOGGER_OSX_ARCH_LIST ";" _CYLOGGER_OSX_ARCH_JOINED)
    endif()
endif()

# For iOS builds, the build script passes TARGET_ARCH (e.g., "arm64", "x86_64",
# "arm64-simulator"). Use TARGET_ARCH for the output directory path to distinguish
# device from simulator builds that both use CMAKE_OSX_ARCHITECTURES=arm64.
# Keep CMAKE_OSX_ARCHITECTURES (from the toolchain) for actual compilation.
if(IOS)
    if(DEFINED TARGET_ARCH AND NOT "${TARGET_ARCH}" STREQUAL "")
        set(CYLOGGER_ARCH "${TARGET_ARCH}")
    elseif(DEFINED CMAKE_OSX_ARCHITECTURES AND NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        set(CYLOGGER_ARCH "${CMAKE_OSX_ARCHITECTURES}")
    else()
        set(CYLOGGER_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()
else()
    # This is the most reliable way to determine the actual target architecture.
    # Note: Do NOT check TARGET_ARCH here — it may be cached from a previous build
    # with a different architecture (e.g., x86 from a prior build persists for x86_64).
    if(WIN32)
        if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(CYLOGGER_ARCH "x86")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
            set(CYLOGGER_ARCH "x86_64")
        else()
            set(CYLOGGER_ARCH "${CMAKE_GENERATOR_PLATFORM}")
        endif()
    elseif(ANDROID)
        # Android derives the architecture from CMAKE_ANDROID_ARCH_ABI
        # Map Android ABI names to simplified architecture names for consistent output paths
        # This matches CYCoroutine's architecture naming convention
        if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
            set(CYLOGGER_ARCH "armeabi-v7a")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
            set(CYLOGGER_ARCH "arm64-v8a")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
            set(CYLOGGER_ARCH "x86")
        elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
            set(CYLOGGER_ARCH "x86_64")
        else()
            set(CYLOGGER_ARCH ${CMAKE_ANDROID_ARCH_ABI})
        endif()
    # Allow overriding the detected architecture via the command line
    elseif(DEFINED CMAKE_SYSTEM_PROCESSOR_OVERRIDE)
        if(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "x86_64|AMD64")
            set(CYLOGGER_ARCH "x86_64")
        elseif(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "i386|i686|x86")
            set(CYLOGGER_ARCH "x86")
        elseif(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "arm64|aarch64")
            if(ANDROID)
                set(CYLOGGER_ARCH "arm64-v8a")
            else()
                set(CYLOGGER_ARCH "arm64")
            endif()
        elseif(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "arm")
            set(CYLOGGER_ARCH "arm")
        else()
            set(CYLOGGER_ARCH ${CMAKE_SYSTEM_PROCESSOR_OVERRIDE})
        endif()
    # Fall back to best-effort detection
    elseif(_CYLOGGER_OSX_ARCH_DEFINED)
        if(_CYLOGGER_OSX_ARCH_JOINED MATCHES "arm64")
            set(CYLOGGER_ARCH "arm64")
        elseif(_CYLOGGER_OSX_ARCH_JOINED MATCHES "x86_64")
            set(CYLOGGER_ARCH "x86_64")
        elseif(_CYLOGGER_OSX_ARCH_JOINED MATCHES "i386|i686|x86")
            set(CYLOGGER_ARCH "x86")
        else()
            list(GET _CYLOGGER_OSX_ARCH_LIST 0 _CYLOGGER_OSX_FIRST_ARCH)
            if(_CYLOGGER_OSX_FIRST_ARCH MATCHES "arm64|aarch64")
                set(CYLOGGER_ARCH "arm64")
            elseif(_CYLOGGER_OSX_FIRST_ARCH MATCHES "x86_64|AMD64")
                set(CYLOGGER_ARCH "x86_64")
            elseif(_CYLOGGER_OSX_FIRST_ARCH MATCHES "i386|i686|x86")
                set(CYLOGGER_ARCH "x86")
            else()
                set(CYLOGGER_ARCH "${_CYLOGGER_OSX_FIRST_ARCH}")
            endif()
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(CYLOGGER_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86")
        set(CYLOGGER_ARCH "x86")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(CYLOGGER_ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        set(CYLOGGER_ARCH "arm")
    else()
        set(CYLOGGER_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif()
endif()

# On macOS ensure the compiler receives the correct architecture flag.
# For iOS, the ios.toolchain.cmake already sets CMAKE_OSX_ARCHITECTURES correctly
# (it maps arm64-simulator -> arm64). Only add -arch for non-iOS Apple builds.
# CYLOGGER_ARCH is used for OUTPUT DIRECTORY paths only, not for -arch flags.
# TARGET_ARCH (e.g., "arm64-simulator") is for path naming only; CMAKE_OSX_ARCHITECTURES
# (e.g., "arm64") is what the compiler uses.
if(APPLE AND NOT IOS AND DEFINED CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${CMAKE_OSX_ARCHITECTURES}")
endif()
if(IOS)
    # TARGET_ARCH may be "arm64-simulator" (path name) but the compiler needs "arm64".
    # The ios.toolchain.cmake already sets CMAKE_OSX_ARCHITECTURES from ARCHS (e.g., "arm64").
    # If TARGET_ARCH is set, ensure CMAKE_OSX_ARCHITECTURES is a valid compiler arch.
    if(DEFINED TARGET_ARCH AND NOT "${TARGET_ARCH}" STREQUAL "")
        # TARGET_ARCH=arm64-simulator -> CMAKE_OSX_ARCHITECTURES should be arm64 for the compiler
        if(TARGET_ARCH STREQUAL "arm64-simulator")
            set(_CYLOGGER_COMPILER_ARCH "arm64")
        else()
            set(_CYLOGGER_COMPILER_ARCH "${TARGET_ARCH}")
        endif()
        # Only override if CMAKE_OSX_ARCHITECTURES doesn't look valid
        if(NOT CMAKE_OSX_ARCHITECTURES MATCHES "^(arm64|x86_64|x86|i386)$")
            set(CMAKE_OSX_ARCHITECTURES "${_CYLOGGER_COMPILER_ARCH}" CACHE STRING "Architectures for compilation" FORCE)
        endif()
    endif()
endif()

# Output directory layout - keep architecture/runtime-specific artifacts separate
set(_CYLOGGER_OUTPUT_BASE "${CMAKE_CURRENT_SOURCE_DIR}/Bin/${CYLOGGER_PLATFORM}/${CYLOGGER_ARCH}")
if(CYLOGGER_RUNTIME_SUFFIX)
    set(_CYLOGGER_OUTPUT_BASE "${_CYLOGGER_OUTPUT_BASE}/${CYLOGGER_RUNTIME_SUFFIX}")
endif()

# Detect if using multi-config generator (Visual Studio, Xcode, etc.)
# Multi-config generators have CMAKE_CONFIGURATION_TYPES set and don't use CMAKE_BUILD_TYPE at configure time
# Single-config generators (Ninja, Make) use CMAKE_BUILD_TYPE
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generator - set base output directories
    # VS will automatically append config name (e.g., Release/) to these paths
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")

    # Set per-configuration output directories for multi-config generators
    # This ensures executables (like Examples) go to the correct output directory
    foreach(_cfg ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${_cfg}" _cfg_upper)
        string(TOLOWER "${_cfg}" _cfg_lower)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_cfg_upper} "${_CYLOGGER_OUTPUT_BASE}/${_cfg_lower}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_cfg_upper} "${_CYLOGGER_OUTPUT_BASE}/${_cfg_lower}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${_cfg_upper} "${_CYLOGGER_OUTPUT_BASE}/${_cfg_lower}")
    endforeach()
else()
    # Single-config generator - use CMAKE_BUILD_TYPE for output directory
    if(CMAKE_BUILD_TYPE)
        string(SUBSTRING "${CMAKE_BUILD_TYPE}" 0 1 _FIRST_CHAR)
        string(SUBSTRING "${CMAKE_BUILD_TYPE}" 1 -1 _REST_CHARS)
        string(TOUPPER "${_FIRST_CHAR}" _FIRST_UPPER)
        set(_CYLOGGER_BUILD_TYPE_CAPITALIZED "${_FIRST_UPPER}${_REST_CHARS}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${_CYLOGGER_BUILD_TYPE_CAPITALIZED}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${_CYLOGGER_BUILD_TYPE_CAPITALIZED}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${_CYLOGGER_BUILD_TYPE_CAPITALIZED}")
    else()
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}")
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CYCoroutine/Inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CYCoroutine/Src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/include)

# CYCommon dependency
option(USE_CYCOMMON "Use CYCommon library" ON)

# CYCommon can be either:
# 1. A bundled library under ThirdParty/CYCommon (local copy, no git submodule)
# 2. A sibling library at the same level as CYLogger's parent
# 3. A sibling library at the grandparent level (when CYLogger is in a ThirdParty subdir)
if(USE_CYCOMMON)
    # First, check if CYCommon exists as a bundled library
    if(NOT DEFINED CYCOMMON_ROOT_DIR)
        # Check relative to CYLogger's location
        get_filename_component(_CYLOGGER_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

        # Check if CYLogger is in a ThirdParty subdirectory of the main project
        # In that case, CYCommon would be at ../../../CYCommon or ../../../../CYCommon
        get_filename_component(_CYLOGGER_PARENT "${_CYLOGGER_DIR}/.." ABSOLUTE)
        get_filename_component(_CYLOGGER_GRANDPARENT "${_CYLOGGER_PARENT}/.." ABSOLUTE)

        # Try various possible CYCommon locations
        set(_CYCOMMON_CANDIDATES
            "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CYCommon"
            "${_CYLOGGER_PARENT}/ThirdParty/CYCommon"
            "${_CYLOGGER_GRANDPARENT}/ThirdParty/CYCommon"
            "${_CYLOGGER_PARENT}/CYCommon"
            "${_CYLOGGER_GRANDPARENT}/CYCommon"
        )

        set(CYCOMMON_ROOT_DIR "")
        foreach(_candidate ${_CYCOMMON_CANDIDATES})
            if(EXISTS "${_candidate}/Build/CMakeLists.txt")
                set(CYCOMMON_ROOT_DIR "${_candidate}")
                message(STATUS "Found CYCommon at ${CYCOMMON_ROOT_DIR}")
                break()
            endif()
        endforeach()

        if(NOT CYCOMMON_ROOT_DIR)
            message(WARNING "CYCommon not found in any expected location. CYCommon dependency will be disabled.")
            set(USE_CYCOMMON OFF)
        endif()
    endif()

    # For iOS, build CYCommon as a subdirectory to ensure it uses the iOS toolchain.
    # For iOS shared builds (BUILD_SHARED_LIBS=ON), we can't use add_subdirectory because
    # CYCommon must be static (iOS dylib policy), but CYLogger_shared is already defined
    # BEFORE this add_subdirectory call. So for shared builds, we set up CYCommon as an
    # IMPORTED library pointing to the pre-built static library.
    if(IOS AND USE_CYCOMMON)
        set(_CYCOMMON_INITIAL_SHARED ${BUILD_SHARED_LIBS})
        # Force CYCommon to build as static for iOS
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
        # Determine the arch for CYCommon output directory.
        # Use TARGET_ARCH passed from the build script (e.g., arm64, arm64-simulator, x86_64).
        if(DEFINED TARGET_ARCH AND NOT "${TARGET_ARCH}" STREQUAL "")
            set(_CYCOMMON_IOS_ARCH "${TARGET_ARCH}")
        else()
            set(_CYCOMMON_IOS_ARCH "arm64")
        endif()
        # Unset and re-set CYCOMMON_OUTPUT_ARCH so it gets through to CYCommon subdirectory.
        # Just using CACHE FORCE doesn't override an already-defined cache variable.
        unset(CYCOMMON_OUTPUT_ARCH CACHE)
        set(CYCOMMON_OUTPUT_ARCH "${_CYCOMMON_IOS_ARCH}" CACHE STRING "CYCommon output arch subdirectory" FORCE)
        set(CYCOMMON_OUTPUT_DIR "${CYCOMMON_ROOT_DIR}/Bin/iOS/${_CYCOMMON_IOS_ARCH}" CACHE PATH "CYCommon output directory" FORCE)
        # CYCommon outputs to Bin/iOS/${_CYCOMMON_IOS_ARCH}/${CMAKE_BUILD_TYPE}/ from standalone build.
        # With CYCOMMON_OUTPUT_DIR_IS_FINAL OFF (default), CMake will append ${CMAKE_BUILD_TYPE}.
        # CYLogger's build_ios.sh passes CYCOMMON_OUTPUT_DIR pointing to the arch dir (no BT).
        unset(CYCOMMON_OUTPUT_DIR_IS_FINAL CACHE)
        # Unset and re-set TARGET_ARCH so CYCommon subdirectory gets the correct value.
        unset(TARGET_ARCH CACHE)
        set(TARGET_ARCH "${_CYCOMMON_IOS_ARCH}" CACHE STRING "Target arch for CYCommon subdirectory build")
        endif()
        # Use the SAVED value (_CYCOMMON_INITIAL_SHARED) for the conditional,
        # since BUILD_SHARED_LIBS has been set to OFF above.
        if(_CYCOMMON_INITIAL_SHARED)
            # iOS shared build: set up CYCommon as an IMPORTED static library
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_CYCOMMON_LIB_NAME "libCYCommonD.a")
            else()
                set(_CYCOMMON_LIB_NAME "libCYCommon.a")
            endif()
            set(_CYCOMMON_LIB_PATH "${CYCOMMON_ROOT_DIR}/Bin/iOS/${_CYCOMMON_IOS_ARCH}/${CMAKE_BUILD_TYPE}/${_CYCOMMON_LIB_NAME}")
            if(EXISTS "${_CYCOMMON_LIB_PATH}")
                if(NOT TARGET CYCommon::static)
                    add_library(CYCommon::static STATIC IMPORTED)
                    set_target_properties(CYCommon::static PROPERTIES
                        IMPORTED_LOCATION "${_CYCOMMON_LIB_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                    )
                endif()
                include_directories(${CYCOMMON_ROOT_DIR}/Inc)
                message(STATUS "CYCommon (iOS shared): ${_CYCOMMON_LIB_PATH}")
            else()
                message(FATAL_ERROR "CYCommon not found: ${_CYCOMMON_LIB_PATH}")
            endif()
        else()
            # iOS static build: use add_subdirectory
            add_subdirectory(${CYCOMMON_ROOT_DIR}/Build)
        endif()
        # Unset CYCOMMON_OUTPUT_DIR_IS_FINAL to prevent cache pollution
        set(CYCOMMON_OUTPUT_DIR_IS_FINAL "" CACHE BOOL "" FORCE)
        # Restore BUILD_SHARED_LIBS for subsequent cmake logic
        set(BUILD_SHARED_LIBS ${_CYCOMMON_INITIAL_SHARED} CACHE BOOL "" FORCE)
    endif()

    # For iOS shared builds (BUILD_SHARED_LIBS=ON), add_subdirectory is skipped.
    # Find CYCommon as a pre-built library at CYCommon/Bin/iOS/<arch>/<BT>/.


    # For non-iOS platforms, search for pre-built CYCommon libraries.
    if(USE_CYCOMMON AND EXISTS "${CYCOMMON_ROOT_DIR}/Inc/CYCommon/CYCommon.hpp" AND NOT IOS)
        # Check for external CYCOMMON_LIB_PATH override first
        if(DEFINED CYCOMMON_LIB_PATH AND EXISTS "${CYCOMMON_LIB_PATH}")
            if(NOT TARGET CYCommon::static)
                add_library(CYCommon::static STATIC IMPORTED)
                set_target_properties(CYCommon::static PROPERTIES
                    IMPORTED_LOCATION "${CYCOMMON_LIB_PATH}"
                    INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                )
            endif()
            include_directories(${CYCOMMON_ROOT_DIR}/Inc)
        elseif(WIN32)
            # Windows: CYCommon is built by build_windows_all.bat before cmake configure.
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_CYCOMMON_LIB_NAME "CYCommonD.lib")
            else()
                set(_CYCOMMON_LIB_NAME "CYCommon.lib")
            endif()
            if(DEFINED CYLOGGER_MSVC_RUNTIME)
                set(_CYCOMMON_RUNTIME_FOLDER "${CYLOGGER_MSVC_RUNTIME}")
            else()
                set(_CYCOMMON_RUNTIME_FOLDER "MD")
            endif()
            # Path format: {ARCH}/{RUNTIME}/{BUILD_TYPE}/CYCommon[D].lib
            set(_CYCOMMON_LOGGER_WIN_LIB "${CYLOGGER_ROOT_DIR}/Bin/Windows/${CYLOGGER_ARCH}/${_CYCOMMON_RUNTIME_FOLDER}/${CMAKE_BUILD_TYPE}/${_CYCOMMON_LIB_NAME}")
            if(EXISTS "${_CYCOMMON_LOGGER_WIN_LIB}")
                set(_CYCOMMON_LIB_PATH "${_CYCOMMON_LOGGER_WIN_LIB}")
            else()
                set(_CYCOMMON_LIB_PATH "")
            endif()
            if(NOT "${_CYCOMMON_LIB_PATH}" STREQUAL "" AND EXISTS "${_CYCOMMON_LIB_PATH}")
                if(NOT TARGET CYCommon::static)
                    add_library(CYCommon::static STATIC IMPORTED)
                    set_target_properties(CYCommon::static PROPERTIES
                        IMPORTED_LOCATION "${_CYCOMMON_LIB_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                    )
                endif()
                include_directories(${CYCOMMON_ROOT_DIR}/Inc)
            else()
                message(WARNING "CYCommon library not found. Build CYCommon first with build_windows_all.bat.")
            endif()
        elseif(DARWIN)
            # macOS: CYCommon is built by ensure_cycommon_macos before cmake configure.
            string(SUBSTRING "${CMAKE_BUILD_TYPE}" 0 1 _FIRST)
            string(SUBSTRING "${CMAKE_BUILD_TYPE}" 1 -1 _REST)
            string(TOUPPER "${_FIRST}" _FIRST_UPPER)
            set(_CYCOMMON_BT_CAP "${_FIRST_UPPER}${_REST}")
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_CYCOMMON_LIB_NAME "libCYCommonD.a")
            else()
                set(_CYCOMMON_LIB_NAME "libCYCommon.a")
            endif()
            set(_CYCOMMON_MACOS_PATH "${CYCOMMON_ROOT_DIR}/Bin/MacOS/${TARGET_ARCH}/${_CYCOMMON_BT_CAP}/${_CYCOMMON_LIB_NAME}")
            if(EXISTS "${_CYCOMMON_MACOS_PATH}")
                if(NOT TARGET CYCommon::static)
                    add_library(CYCommon::static STATIC IMPORTED)
                    set_target_properties(CYCommon::static PROPERTIES
                        IMPORTED_LOCATION "${_CYCOMMON_MACOS_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                    )
                endif()
                include_directories(${CYCOMMON_ROOT_DIR}/Inc)
            else()
                message(WARNING "CYCommon library not found at expected macOS paths.")
            endif()
        elseif(ANDROID)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_CYCOMMON_LIB_NAME "libCYCommonD.a")
            else()
                set(_CYCOMMON_LIB_NAME "libCYCommon.a")
            endif()
            set(_CYCOMMON_ANDROID_PATH "${CYCOMMON_ROOT_DIR}/Bin/Android/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE}/${_CYCOMMON_LIB_NAME}")
            if(EXISTS "${_CYCOMMON_ANDROID_PATH}")
                set(_CYCOMMON_LIB_PATH "${_CYCOMMON_ANDROID_PATH}")
            else()
                set(_CYCOMMON_ANDROID_FALLBACK "${CYCOMMON_ROOT_DIR}/Bin/Android/${CMAKE_ANDROID_ARCH_ABI}/${CMAKE_BUILD_TYPE}/${_CYCOMMON_LIB_NAME}")
                if(EXISTS "${_CYCOMMON_ANDROID_FALLBACK}")
                    set(_CYCOMMON_LIB_PATH "${_CYCOMMON_ANDROID_FALLBACK}")
                else()
                    set(_CYCOMMON_LIB_PATH "")
                endif()
            endif()
            if(NOT "${_CYCOMMON_LIB_PATH}" STREQUAL "" AND EXISTS "${_CYCOMMON_LIB_PATH}")
                if(NOT TARGET CYCommon::static)
                    add_library(CYCommon::static STATIC IMPORTED)
                    set_target_properties(CYCommon::static PROPERTIES
                        IMPORTED_LOCATION "${_CYCOMMON_LIB_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                    )
                endif()
                include_directories(${CYCOMMON_ROOT_DIR}/Inc)
            else()
                message(WARNING "CYCommon library not found at expected Android paths.")
            endif()
        elseif(UNIX AND NOT APPLE)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set(_CYCOMMON_LIB_NAME "libCYCommonD.a")
            else()
                set(_CYCOMMON_LIB_NAME "libCYCommon.a")
            endif()
            string(SUBSTRING "${CMAKE_BUILD_TYPE}" 0 1 _FIRST)
            string(SUBSTRING "${CMAKE_BUILD_TYPE}" 1 -1 _REST)
            string(TOUPPER "${_FIRST}" _FIRST_UPPER)
            set(_CYLOGGER_BT_CAP "${_FIRST_UPPER}${_REST}")
            set(_CYCOMMON_LINUX_PATH "${CYLOGGER_ROOT_DIR}/Bin/Linux/${CYLOGGER_ARCH}/${_CYLOGGER_BT_CAP}/${_CYCOMMON_LIB_NAME}")
            if(EXISTS "${_CYCOMMON_LINUX_PATH}")
                set(_CYCOMMON_LIB_PATH "${_CYCOMMON_LINUX_PATH}")
            else()
                set(_CYCOMMON_LINUX_FALLBACK "${CYCOMMON_ROOT_DIR}/Bin/Linux/${CYLOGGER_ARCH}/${_CYLOGGER_BT_CAP}/${_CYCOMMON_LIB_NAME}")
                if(EXISTS "${_CYCOMMON_LINUX_FALLBACK}")
                    set(_CYCOMMON_LIB_PATH "${_CYCOMMON_LINUX_FALLBACK}")
                else()
                    set(_CYCOMMON_LIB_PATH "")
                endif()
            endif()
            if(NOT "${_CYCOMMON_LIB_PATH}" STREQUAL "" AND EXISTS "${_CYCOMMON_LIB_PATH}")
                if(NOT TARGET CYCommon::static)
                    add_library(CYCommon::static STATIC IMPORTED)
                    set_target_properties(CYCommon::static PROPERTIES
                        IMPORTED_LOCATION "${_CYCOMMON_LIB_PATH}"
                        INTERFACE_INCLUDE_DIRECTORIES "${CYCOMMON_ROOT_DIR}/Inc"
                    )
                endif()
                include_directories(${CYCOMMON_ROOT_DIR}/Inc)
            else()
                message(WARNING "CYCommon library not found at expected Linux paths.")
            endif()
        endif()
    endif()

    # For iOS, CYCommon is built via add_subdirectory above (when IOS AND USE_CYCOMMON).
    # Ensure CYCommon::static target is available for CYLogger_shared.
    if(IOS AND USE_CYCOMMON)
        include_directories(${CYCOMMON_ROOT_DIR}/Inc)
        add_definitions(-DUSE_CYCOMMON)
    endif()

    option(CYLOGGER_BUILD_IOS_DYLIB "Build CYLogger dylib for iOS (even when BUILD_SHARED_LIBS is OFF)" OFF)
    if(CYLOGGER_BUILD_IOS_DYLIB)
        set(_CYLOGGER_FORCE_SHARED ON)
    endif()

# Build toggles
# CYLOGGER_WANT_SHARED controls whether to build the CYLogger_shared target.
# Use the saved BUILD_SHARED_LIBS value (captured after project() call, before any
# subdirectory may reset it) as the default. If user passed -DCYLOGGER_WANT_SHARED=ON
# on the command line, that takes precedence.
if(DEFINED CYLOGGER_WANT_SHARED)
    set(CYLOGGER_WANT_SHARED ${CYLOGGER_WANT_SHARED} CACHE BOOL "Build CYLogger shared library")
else()
    set(CYLOGGER_WANT_SHARED ${_CYLOGGER_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "Build CYLogger shared library")
endif()
option(CYLOGGER_SKIP_SHARED "Skip building CYLogger_shared target" OFF)
# For iOS with CYLOGGER_BUILD_IOS_DYLIB=ON, we need CYLogger_shared
if(_CYLOGGER_FORCE_SHARED)
    set(CYLOGGER_WANT_SHARED ON CACHE BOOL "Build CYLogger shared library" FORCE)
endif()
option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(USE_CYCOROUTINE "Use CYCoroutine library" ON)

# For iOS with CYLOGGER_BUILD_IOS_DYLIB=ON, we need to build CYLogger_shared
# but fmt must be static. Save initial value for later.
set(_CYLOGGER_INITIAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# Add fmt library for std::format support
# Force fmt to build as static library to avoid linking issues
set(FMT_MASTER_PROJECT OFF CACHE BOOL "fmt master project" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "Install fmt" FORCE)
set(FMT_TEST OFF CACHE BOOL "Build fmt tests" FORCE)
set(FMT_DOC OFF CACHE BOOL "Build fmt documentation" FORCE)

# Use lowercase 'd' for Debug builds to match expected CYLogger naming (libfmtd.a)
set(FMT_DEBUG_POSTFIX "d" CACHE STRING "Debug library postfix" FORCE)

# Set position-independent code for fmt to allow linking into shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Build position-independent code" FORCE)

# Add fmt as a subdirectory
add_subdirectory(ThirdParty/fmt)

# fmt's project() resets output directories, so set them again afterwards
set_target_properties(fmt PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
)

# Restore BUILD_SHARED_LIBS for CYLogger (fmt enforces static build)
# Use CYLOGGER_WANT_SHARED as the source of truth since it survives the CMake 4.2 bug
if(CYLOGGER_WANT_SHARED)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
else()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
endif()

# Add definition to use fmt instead of std::format
add_definitions(-DUSE_FMT_LIBRARY)

# Optionally build the bundled CYCoroutine dependency
if(USE_CYCOROUTINE)
    # Check if CYCOROUTINE_LIB_PATH is provided (pre-built library)
    if(DEFINED CYCOROUTINE_LIB_PATH AND EXISTS "${CYCOROUTINE_LIB_PATH}")
        message(STATUS "Using pre-built CYCoroutine library: ${CYCOROUTINE_LIB_PATH}")
        # Create imported or alias targets for CYCoroutine when a pre-built library is supplied.
        # Keep the canonical `CYCoroutine::static` target so downstream code can rely on one name.
        if(NOT TARGET CYCoroutine::static)
            add_library(CYCoroutine::static STATIC IMPORTED)
            set_target_properties(CYCoroutine::static PROPERTIES
                IMPORTED_LOCATION "${CYCOROUTINE_LIB_PATH}"
            )
            if(EXISTS "${CMAKE_SOURCE_DIR}/ThirdParty/CYCoroutine/Inc")
                set_target_properties(CYCoroutine::static PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/ThirdParty/CYCoroutine/Inc"
                )
            endif()
        endif()
        if(NOT TARGET CYCoroutine_static)
            add_library(CYCoroutine_static ALIAS CYCoroutine::static)
        endif()
        # Mark that CYCoroutine is pre-built so install rules skip it
        set(_CYCOROUTINE_PREBUILT TRUE)
        set(_CYCOROUTINE_SKIP_BUILD TRUE)
    else()
        # CYCoroutine only supports static library on Windows and iOS
        # Save the current BUILD_SHARED_LIBS value for CYLogger
        set(CYLOGGER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

        # On iOS, build CYCoroutine as both static and shared (shared for universal dylib).
        # On Windows, force static library only.
        # On other platforms (macOS, Linux, Android), allow the same library type as CYLogger.
        if(WIN32)
            set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
            set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
        elseif(IOS)
            set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
            set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
        else()
            set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
        endif()

        # Pass TARGET_ARCH to CYCoroutine for consistent output directory naming
        # This ensures CYCoroutine outputs to the same directory structure as CYLogger
        if(IOS)
            set(TARGET_ARCH_FOR_COROUTINE "${TARGET_ARCH}")
        elseif(DEFINED TARGET_ARCH)
            set(TARGET_ARCH_FOR_COROUTINE "${TARGET_ARCH}")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
            set(TARGET_ARCH_FOR_COROUTINE "x86")
        elseif(CMAKE_GENERATOR_PLATFORM STREQUAL "x64" OR CMAKE_GENERATOR_PLATFORM STREQUAL "")
            set(TARGET_ARCH_FOR_COROUTINE "x86_64")
        else()
            set(TARGET_ARCH_FOR_COROUTINE "${CMAKE_GENERATOR_PLATFORM}")
        endif()
        # Pass the runtime library setting to CYCoroutine
        if(MSVC)
            set(WINDOWS_RUNTIME ${CYLOGGER_MSVC_RUNTIME} CACHE STRING "Windows runtime library" FORCE)
        endif()
        # Set TARGET_ARCH for CYCoroutine's output directory resolution
        set(TARGET_ARCH "${TARGET_ARCH_FOR_COROUTINE}" CACHE STRING "Target architecture for CYCoroutine")
        # Set output directories for CYCoroutine so its dylibs go to CYLogger's Bin/iOS directory
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" CACHE PATH "CYCoroutine dylib output")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}" CACHE PATH "CYCoroutine static output")
        add_subdirectory(ThirdParty/CYCoroutine/Build)

        # Restore BUILD_SHARED_LIBS for CYLogger
        set(BUILD_SHARED_LIBS ${CYLOGGER_BUILD_SHARED_LIBS} CACHE BOOL "Build shared libraries" FORCE)

        add_definitions(-DUSE_CYCOROUTINE)
        set(_CYCOROUTINE_SKIP_BUILD FALSE)
    endif()
else()
    set(_CYCOROUTINE_SKIP_BUILD TRUE)
endif()

# Add project subdirectories
add_subdirectory(Src)

if(BUILD_EXAMPLES)
    add_subdirectory(Example)
endif()

# Install headers and configuration scripts
include(GNUInstallDirs)

install(DIRECTORY Inc/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export the CMake package configuration
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CYLoggerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CYLoggerConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CYLogger
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CYLoggerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CYLoggerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CYLoggerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CYLogger
)