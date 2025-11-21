# 示例项目
add_subdirectory(CYLoggerConsoleTest)

# 简化版示例，不依赖CYCoroutine
add_subdirectory(CYLoggerSimpleTest)

# 只在Windows平台添加GUI示例
if(WIN32)
    add_subdirectory(CYLoggerTest)
endif()