# 控制台测试示例
add_executable(CYLoggerConsoleTest CYLoggerConsoleTest.cpp)

# 链接CYLogger库
target_link_libraries(CYLoggerConsoleTest PRIVATE CYLogger)

# 如果使用动态库，定义宏
if(BUILD_SHARED_LIBS)
    target_compile_definitions(CYLoggerConsoleTest PRIVATE CYLOGGER_USE_DLL)
endif()

# 平台特定设置
if(WIN32)
    target_link_libraries(CYLoggerConsoleTest PRIVATE)
endif()

# 设置输出目录
set_target_properties(CYLoggerConsoleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)