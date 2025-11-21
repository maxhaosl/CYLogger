cmake_minimum_required(VERSION 3.16)
project(CYLogger VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 确保所有编译器使用C++20
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
endif()

# 设置构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 平台检测
if(WIN32)
    set(CYLOGGER_PLATFORM "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
elseif(APPLE)
    if(IOS)
        set(CYLOGGER_PLATFORM "iOS")
        # iOS最低支持版本
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        endif()
    else()
        set(CYLOGGER_PLATFORM "macOS")
        # macOS最低支持版本
        if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        endif()
    endif()
elseif(ANDROID)
    set(CYLOGGER_PLATFORM "Android")
elseif(UNIX)
    set(CYLOGGER_PLATFORM "Linux")
endif()

# 处理器架构检测
if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    set(CYLOGGER_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    set(CYLOGGER_ARCH "x64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i386|i686|x86")
    set(CYLOGGER_ARCH "x86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(CYLOGGER_ARCH "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(CYLOGGER_ARCH "arm")
else()
    set(CYLOGGER_ARCH ${CMAKE_SYSTEM_PROCESSOR})
endif()

# 输出目录设置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CYLOGGER_PLATFORM}/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CYLOGGER_PLATFORM}/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin/${CYLOGGER_PLATFORM}/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE})

# 包含目录
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Inc)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/CYCoroutine/Inc)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/CYCoroutine/Src)

# 选项
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_EXAMPLES "Build examples" ON)

# 首先构建CYCoroutine依赖
add_subdirectory(ThirdParty/CYCoroutine/Build)

# 添加子目录
add_subdirectory(Src)

if(BUILD_EXAMPLES)
    add_subdirectory(Example)
endif()

# 安装配置
include(GNUInstallDirs)

install(DIRECTORY Inc/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 导出配置
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