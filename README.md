# CYLogger

C++20 跨平台日志库，支持协程

基于C++20实现的日志库，支持协程、独立锁、双缓冲队列、格式化、范围、模板、约束等特性。

## 特性

- 基于C++20协程实现
- 双缓冲队列设计
- 独立锁机制
- 日志模式过滤器
- 日志模板布局
- 异常处理
- 接口简单易用，支持跨平台
- 支持多种日志级别：TRACE、DEBUG、INFO、WARN、ERROR、FATAL
- 支持多种输出方式：控制台、文件、系统日志等

## 支持平台

- Windows (x86, x64)
- macOS (x64, arm64)
- iOS (arm64)
- Linux (x86, x64)
- Android (armeabi-v7a, arm64-v8a, x86, x86_64)

## 依赖项

- CYCoroutine (内置)

## 编译要求

- CMake 3.16+
- C++20兼容编译器
- Android: NDK r26b或更高版本

## 快速开始

### Windows 示例

```cpp
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "ICYLogger.hpp"

int main()
{
    TCHAR szFilePath[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, szFilePath, sizeof(szFilePath));
    (_tcsrchr(szFilePath, _T('\\')))[1] = 0;

    CY_LOG_CONFIG(szFilePath, LOG_SHOW_CONSOLE_WINDOW, LOG_LAYOUT_TYPE);

    char szBuffer[256] = { 1 };

    int nLineCount = 0;
    for (int i = 0; i < 10; i++)
    {
        CY_LOG_TRACE(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_DEBUG(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_INFO(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_WARN(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ERROR(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_FATAL(_T("%d - %s"), nLineCount, _T("This is a test message!"));

        CY_LOG_ESCAPE_TRACE(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_DEBUG(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_INFO(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_WARN(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_ERROR(_T("%d - %s"), nLineCount, _T("This is a test message!"));
        CY_LOG_ESCAPE_FATAL(_T("%d - %s"), nLineCount, _T("This is a test message!"));

        CY_LOG_DIRECT_TRACE(_T("This is a test message!"));
        CY_LOG_HEX_TRACE(szBuffer, sizeof(szBuffer));
    }

    CY_LOG_FREE();

    return 0;
}
```

### 跨平台示例

```cpp
#include "ICYLogger.hpp"

int main()
{
    // 初始化日志系统
    CYLogger::GetInstance().Initialize("MyApp", CYLogger::LogLevel::Info);
    
    // 添加文件输出器
    auto fileAppender = std::make_shared<CYLogger::CYLoggerFileAppender>("log.txt");
    CYLogger::GetInstance().AddAppender(fileAppender);
    
    // 添加控制台输出器
    auto consoleAppender = std::make_shared<CYLogger::CYLoggerConsoleAppender>();
    CYLogger::GetInstance().AddAppender(consoleAppender);
    
    // 记录日志
    CY_LOG_INFO("Application started");
    CY_LOG_WARN("This is a warning message");
    CY_LOG_ERROR("This is an error message");
    
    // 清理
    CYLogger::GetInstance().Shutdown();
    
    return 0;
}
```

## 编译

### 使用构建脚本

#### Windows
```bash
# 编译静态库
./Build/build_windows.bat

# 编译动态库
./Build/build_windows.bat SHARED
```

#### macOS
```bash
# 编译静态库
./Build/build_macos.sh

# 编译动态库
./Build/build_macos.sh SHARED
```

#### Linux
```bash
# 编译静态库
./Build/build_linux.sh

# 编译动态库
./Build/build_linux.sh SHARED
```

#### Android
```bash
# 设置NDK路径
export ANDROID_NDK_HOME=/path/to/android-ndk

# 编译所有架构的静态库
./Build/build_android_all.sh Release Static

# 编译所有架构的动态库
./Build/build_android_all.sh Release SHARED

# 编译特定架构
./Build/build_android.sh Release SHARED arm64-v8a 31
```

### 使用CMake

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

## 库文件

编译后的库文件位于 `Bin` 目录下，按平台和架构组织：

```
Bin/
├── Windows/
│   ├── x86/
│   └── x64/
├── macOS/
│   ├── x64/
│   └── arm64/
├── iOS/
│   └── arm64/
├── Linux/
│   ├── x86/
│   └── x64/
└── Android/
    ├── armeabi-v7a/
    ├── arm64-v8a/
    ├── x86/
    └── x86_64/
```

## 许可证

本项目采用 MIT 许可证。详情请参阅 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request！

## 更新日志

详见 Change.log 文件。

