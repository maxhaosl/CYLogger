# GUI测试示例 (仅Windows)
if(WIN32)
    # 收集资源文件
    set(RESOURCES
        CYLoggerTest.rc
        res/CYLoggerTest.ico
    )

    # 创建可执行文件
    add_executable(CYLoggerTest
        CYLoggerTest.cpp
        CYLoggerTestDlg.cpp
        CYLoggerTest.h
        CYLoggerTestDlg.h
        framework.h
        pch.cpp
        pch.h
        targetver.h
        ${RESOURCES}
    )

    # 链接CYLogger库
    target_link_libraries(CYLoggerTest PRIVATE CYLogger)

    # 如果使用动态库，定义宏
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(CYLoggerTest PRIVATE CYLOGGER_USE_DLL)
    endif()

    # Windows特定链接库
    target_link_libraries(CYLoggerTest PRIVATE
        comctl32
        gdi32
        user32
        kernel32
        shell32
    )

    # 预编译头设置
    target_compile_definitions(CYLoggerTest PRIVATE
        WIN32
        _WINDOWS
        _CRT_SECURE_NO_WARNINGS
    )

    # 设置输出目录
    set_target_properties(CYLoggerTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()