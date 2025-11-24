cmake_minimum_required(VERSION 3.16)
project(CYLogger VERSION 1.0.0 LANGUAGES CXX)

# Configure the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always force the compiler to use C++20
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
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

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform detection
if(WIN32)
    set(CYLOGGER_PLATFORM "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(APPLE)
    if(IOS)
        set(CYLOGGER_PLATFORM "iOS")
        # Minimum iOS deployment target
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        endif()
    else()
        set(CYLOGGER_PLATFORM "macOS")
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

if(DEFINED TARGET_ARCH)
    # Respect the explicitly requested architecture
    # Map x64 to x86_64 for consistent output paths on Windows
    if(TARGET_ARCH STREQUAL "x64")
        set(CYLOGGER_ARCH "x86_64")
    else()
        set(CYLOGGER_ARCH ${TARGET_ARCH})
    endif()
elseif(ANDROID)
    # Android derives the architecture from CMAKE_ANDROID_ARCH_ABI
    # Map Android ABI names to simplified architecture names for consistent output paths
    # This matches CYCoroutine's architecture naming convention
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
        set(CYLOGGER_ARCH "armv7")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(CYLOGGER_ARCH "arm64")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
        set(CYLOGGER_ARCH "x86")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        set(CYLOGGER_ARCH "x86_64")
    else()
        # Fallback to the original ABI name if not recognized
        set(CYLOGGER_ARCH ${CMAKE_ANDROID_ARCH_ABI})
    endif()
# Allow overriding the detected architecture via the command line
elseif(DEFINED CMAKE_SYSTEM_PROCESSOR_OVERRIDE)
    if(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "x86_64|AMD64")
        set(CYLOGGER_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "i386|i686|x86")
        set(CYLOGGER_ARCH "x86")
    elseif(CMAKE_SYSTEM_PROCESSOR_OVERRIDE MATCHES "arm64|aarch64")
        set(CYLOGGER_ARCH "arm64")
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

# On macOS ensure the compiler receives the correct architecture flag
if(APPLE AND DEFINED CYLOGGER_ARCH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ${CYLOGGER_ARCH}")
endif()

# Output directory layout - keep architecture/runtime-specific artifacts separate
set(_CYLOGGER_OUTPUT_BASE "${CMAKE_CURRENT_SOURCE_DIR}/Bin/${CYLOGGER_PLATFORM}/${CYLOGGER_ARCH}")
if(CYLOGGER_RUNTIME_SUFFIX)
    set(_CYLOGGER_OUTPUT_BASE "${_CYLOGGER_OUTPUT_BASE}/${CYLOGGER_RUNTIME_SUFFIX}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${CMAKE_BUILD_TYPE}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_CYLOGGER_OUTPUT_BASE}/${CMAKE_BUILD_TYPE}")

# Multi-config generators (Visual Studio, Xcode, etc.)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${_CYLOGGER_OUTPUT_BASE}/${OUTPUTCONFIG}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${_CYLOGGER_OUTPUT_BASE}/${OUTPUTCONFIG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${_CYLOGGER_OUTPUT_BASE}/${OUTPUTCONFIG}")
endforeach()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CYCoroutine/Inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/CYCoroutine/Src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/fmt/include)

# Build toggles
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(USE_CYCOROUTINE "Use CYCoroutine library" ON)

set(_CYLOGGER_INITIAL_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

# Add fmt library for std::format support
# Force fmt to build as static library to avoid linking issues
set(FMT_MASTER_PROJECT OFF CACHE BOOL "fmt master project" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "Install fmt" FORCE)
set(FMT_TEST OFF CACHE BOOL "Build fmt tests" FORCE)
set(FMT_DOC OFF CACHE BOOL "Build fmt documentation" FORCE)

# Add fmt as a subdirectory
add_subdirectory(ThirdParty/fmt)

# Restore BUILD_SHARED_LIBS for CYLogger (fmt enforces static build)
set(BUILD_SHARED_LIBS ${_CYLOGGER_INITIAL_BUILD_SHARED_LIBS} CACHE BOOL "Build shared libraries" FORCE)

# Add definition to use fmt instead of std::format
add_definitions(-DUSE_FMT_LIBRARY)

# Optionally build the bundled CYCoroutine dependency
if(USE_CYCOROUTINE)
    # CYCoroutine only supports static library on Windows, but supports both static and shared on other platforms
    # Save the current BUILD_SHARED_LIBS value for CYLogger
    set(CYLOGGER_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    
    # On Windows, force static library for CYCoroutine
    # On other platforms (macOS, Linux, Android), allow the same library type as CYLogger
    if(WIN32)
        # Windows: CYCoroutine only supports static library
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
        set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
    else()
        # Other platforms: CYCoroutine can be built as shared or static, matching CYLogger's build type
        set(BUILD_STATIC_LIBS ON CACHE BOOL "Build static libraries" FORCE)
    endif()
    
    set(CYLOGGER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    # Pass the runtime library setting to CYCoroutine
    if(MSVC)
        set(WINDOWS_RUNTIME ${CYLOGGER_MSVC_RUNTIME} CACHE STRING "Windows runtime library" FORCE)
    endif()
    add_subdirectory(ThirdParty/CYCoroutine/Build)
    
    # Restore BUILD_SHARED_LIBS for CYLogger
    set(BUILD_SHARED_LIBS ${CYLOGGER_BUILD_SHARED_LIBS} CACHE BOOL "Build shared libraries" FORCE)
    
    add_definitions(-DUSE_CYCOROUTINE)
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