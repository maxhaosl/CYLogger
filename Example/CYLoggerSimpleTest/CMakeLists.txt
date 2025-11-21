# 简化版示例项目，不依赖CYCoroutine

# 收集源文件
set(SIMPLE_TEST_SOURCES
    CYLoggerSimpleTest.cpp
)

# 创建可执行文件
add_executable(CYLoggerSimpleTest ${SIMPLE_TEST_SOURCES})

# 添加头文件目录
target_include_directories(CYLoggerSimpleTest PRIVATE 
    ${CMAKE_SOURCE_DIR}/Inc
    ${CMAKE_SOURCE_DIR}/Src
)

# 链接CYLogger库
target_link_libraries(CYLoggerSimpleTest PRIVATE CYLogger)

# 设置输出目录
set_target_properties(CYLoggerSimpleTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin/${CYLOGGER_ARCH}/${CMAKE_BUILD_TYPE}
)