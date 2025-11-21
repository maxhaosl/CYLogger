# 收集所有源文件
file(GLOB_RECURSE CYLOGGER_SOURCES
    "*.cpp"
    "*.hpp"
)

# 排除示例文件
list(FILTER CYLOGGER_SOURCES EXCLUDE REGEX ".*Example.*")

# 创建库目标
add_library(CYLogger ${CYLOGGER_SOURCES})

# 添加Src目录到include路径
target_include_directories(CYLogger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标属性
set_target_properties(CYLogger PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "../Inc/ICYLogger.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerDefine.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerPatternFilter.hpp"
    PUBLIC_HEADER "../Inc/ICYLoggerTemplateLayout.hpp"
)

# 编译定义
target_compile_definitions(CYLogger PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:CYLOGGER_USE_DLL>
    $<$<BOOL:${BUILD_SHARED_LIBS}>:CYLOGGER_EXPORTS>
)

# 平台特定的编译定义
if(WIN32)
    target_compile_definitions(CYLogger PRIVATE
        WIN32
        $<$<CONFIG:Debug>:_DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
        _LIB
    )
elseif(APPLE)
    if(IOS)
        target_compile_definitions(CYLogger PRIVATE CYLOGGER_IOS_OS)
    else()
        target_compile_definitions(CYLogger PRIVATE CYLOGGER_MAC_OS)
    endif()
elseif(ANDROID)
    target_compile_definitions(CYLogger PRIVATE CYLOGGER_ANDROID_OS)
elseif(UNIX)
    target_compile_definitions(CYLogger PRIVATE CYLOGGER_LINUX_OS)
endif()

# 链接库
find_package(Threads REQUIRED)
target_link_libraries(CYLogger PUBLIC Threads::Threads CYCoroutine)

# Windows特定设置
if(WIN32)
    target_link_libraries(CYLogger PUBLIC ws2_32)
endif()

# 安装规则
install(TARGETS CYLogger
    EXPORT CYLoggerTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# 导出目标
install(EXPORT CYLoggerTargets
    FILE CYLoggerTargets.cmake
    NAMESPACE CYLogger::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CYLogger
)