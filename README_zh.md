[English](README.md) | [中文](README_zh.md)

# CYLogger

CYLogger 是一个基于 C++20 协程的跨平台日志库。它提供协程感知的追加器、无锁缓冲区、灵活的格式化，以及可移植的构建脚本，为每个支持的平台提供静态库和共享库。

## 特性

- 基于 C++20 协程的日志管道，集成 CYCoroutine
- 锁隔离的双缓冲队列，确保确定性吞吐量
- 丰富的模式/布局/过滤器系统，支持基于模板的自定义
- 多种追加器：控制台、文件、远程、系统日志、zip 轮转等
- 内置异常处理、统计信息和 FPS 计数器，用于性能分析
- 统一的二进制布局：`Bin/<Platform>/<Arch>[/<CRT>]/<Config>`

## 支持的平台

- Windows: x64 (Visual Studio 或 MinGW)
- macOS: arm64、x86_64 和通用切片（静态和动态）
- iOS: arm64、x86_64 模拟器、arm64 模拟器，以及通用切片
- Linux: x86_64、x86（针对 32 位时需要 multilib）
- Android: armeabi-v7a、arm64-v8a、x86、x86_64

## 依赖项

- [CYCoroutine](ThirdParty/CYCoroutine) – 已捆绑并自动构建。在 Windows 上，仅生成静态库（`CYCoroutine.lib`）；在其他平台上，静态库和共享库均可用。其 macOS/iOS 辅助工具现在在 `ThirdParty/CYCoroutine/Bin/<platform>/universal/<config>` 下生成通用切片，因此 CYLogger 在准备依赖项时不再需要单独的 `lipo` 步骤。
- CMake 3.16+
- 兼容 C++20 的工具链（MSVC 19.3x、Clang 14+、GCC 11+）
- Linux 构建默认使用 `clang-17`/`clang++-17`；如果需要不同的编译器，可使用 `CYLOGGER_CC` / `CYLOGGER_CXX` 覆盖。
- Android NDK r26b+（从常见 SDK 文件夹自动检测）

## 构建要求

- macOS/iOS: Xcode 命令行工具、`lipo`
- Linux: `build-essential`、`cmake`，针对 `x86` 可选 `gcc-multilib`
- Android: Android SDK + NDK（自动检测，无需手动导出路径）
- Windows: Visual Studio 2019/2022（带 CMake），或 MSYS2/MinGW shell

## 构建脚本概览 (`Build/`)

| 脚本 | 描述 | 参数 |
| --- | --- | --- |
| `build_all_platforms.sh` | 矩阵构建器，拼接每个平台/架构/配置，生成通用 Apple 二进制文件，如果缺少则自动构建 CYCoroutine。 | `--platforms`, `--build-types`, `--shared-kinds`, `--linux-arches`, `--android-abis`, `--android-api-level`, `--force-linux` |
| `build_all.sh` | 基于主机的包装器，根据 `uname` 委托给正确的单平台辅助脚本。 | `[build_type] [Static|Shared]` |
| `build_macos.sh` | 构建 macOS 静态/共享切片（arm64/x86_64）。 | `[build_type] [Static|Shared]` |
| `build_ios.sh` | 构建 iOS 切片（设备、模拟器、模拟器-arm64）。 | `[build_type] [Static|Shared] [arch]` |
| `build_linux.sh` | 为请求的架构构建 Linux 静态/共享归档文件。 | `[build_type] [Static|Shared] [arch]` |
| `build_linux_all.sh` | 在所有架构、构建类型、库类型上循环调用 `build_linux.sh`。 | `[build_types_csv] [lib_types_csv] [arches_csv]` |
| `build_windows.bat` | 为选定的架构/运行时构建 Windows 库。 | `[build_type] [Static|Shared] [arch] [MD|MT]` |
| `build_windows_all.bat` | 为每个 Windows 架构/构建/库/CRT 组合的矩阵构建器。 | `[arches_csv] [lib_types_csv] [build_types_csv] [crt_csv]` |
| `build_android.sh` | 构建一个 Android ABI。自动检测 SDK/NDK。 | `[build_type] [Static|Shared] [abi] [api_level]` |
| `build_android_all.sh` | 便利包装器，循环所有 Android ABI。 | `[build_type] [Static|Shared] [api_level]` |
| `list_android_libs.sh` | 列出所有已生成的 Android 产物。 | _无_ |

所有脚本在首次失败时退出（`set -euo pipefail`），并尽早显示缺少依赖项的错误。

## `build_all_platforms.sh`

这是需要在 `Bin/` 中获取每个平台输出时的主要入口点。关键行为：

- 自动检查每个切片是否存在 CYCoroutine 产物；缺少的会在构建 CYLogger 之前触发内联 `cmake --build ... --target CYCoroutine_{static,shared}`。
- 从标准位置（`~/Library/Android/sdk`、`$ANDROID_HOME`、`/usr/local/share/android-sdk` 等）自动检测 `ANDROID_SDK_ROOT` / `ANDROID_NDK_HOME`。
- 生成 macOS/iOS 通用库（`lipo`），涵盖静态（`.a`）和共享（`.dylib`）产物。
- 通过 CLI 标志和环境覆盖支持选择性执行。

### CLI 标志

```
./Build/build_all_platforms.sh [options]

--platforms macos,ios,linux,android,windows   限制运行的矩阵（默认：全部）。
--build-types Release,Debug                    CMake 构建类型（默认：Release,Debug）。
--shared-kinds static,shared                  库类型；映射到 BUILD_SHARED_LIBS（默认：两者）。
--linux-arches x86_64,x86                    要编译的 Linux 目标。
--android-abis arm64-v8a,armeabi-v7a,...     要编译的 Android ABI。
--android-api-level 34                        默认 API 级别；强制执行 ABI 特定的最小值。
--force-linux                                 当存在交叉工具链时，允许在非 Linux 主机上构建 Linux。
-h, --help                                    打印用法。
```

环境覆盖：`MACOS_DEPLOYMENT_TARGET`、`IOS_DEPLOYMENT_TARGET`、`ANDROID_API_LEVEL`、`BUILD_JOBS`。

### 使用示例

构建当前主机支持的所有内容：

```bash
./Build/build_all_platforms.sh
```

仅构建 Apple 平台，使用 Release 静态库：

```bash
./Build/build_all_platforms.sh \
  --platforms macos,ios \
  --build-types Release \
  --shared-kinds static
```

仅构建 Android，使用减少的 ABI 集：

```bash
./Build/build_all_platforms.sh \
  --platforms android \
  --android-abis arm64-v8a,x86_64 \
  --build-types Release,Debug
```

从 Docker 容器构建 Linux 矩阵（在 macOS 主机上很有用）：

```bash
docker run --rm -it \
  -v "$(pwd)":/workspace -w /workspace \
  ubuntu:22.04 \
  /bin/bash -lc "apt update && apt install -y build-essential cmake ninja-build && \
                 ./Build/build_all_platforms.sh --platforms linux"
```

## 平台特定辅助脚本

- `build_macos.sh [Release|Debug] [Static|Shared]`: 使用 `CMAKE_OSX_ARCHITECTURES`（默认 arm64;x86_64）构建一种类型，并在可用时运行控制台测试；CYCoroutine 的 Apple 辅助工具现在并行提供现成的通用库。
- `build_ios.sh [Release|Debug] [Static|Shared] [arm64|x86_64|arm64-simulator]`: 映射到适当的 ios-cmake 平台标记（`OS64`、`SIMULATOR64`、`SIMULATORARM64`），并在存在多个切片时受益于 CYCoroutine 的自动通用输出。
- `build_linux.sh [Release|Debug] [Static|Shared] [x86_64|x86]`: 规范化架构名称（aarch64→arm64、amd64→x86_64），传递 `-DCMAKE_SYSTEM_PROCESSOR_OVERRIDE`，根据目标架构自动添加 `-m32`/`-m64` 编译器标志，并自动检测 `clang-17` 工具链（可通过 `CYLOGGER_CC`/`CYLOGGER_CXX` 覆盖）。
- `build_linux_all.sh [build_types_csv] [lib_types_csv] [arches_csv]`: 在提供的逗号分隔列表上迭代 `build_linux.sh`（默认为 `Release,Debug`、`Static,Shared`、`x86_64,x86`）。在构建 CYLogger 之前自动确保使用正确的输出路径构建 CYCoroutine 依赖项，并将检测到的编译器对传播到每个依赖项构建以保持 ABI 兼容性。
- `build_windows.bat [Release|Debug] [Static|Shared] [x64|Win32|arm64] [MD|MT]`: 将架构/运行时转发给 CMake，并在 `Bin/Windows/<arch>/<crt>/<config>` 下发出产物。
- `build_windows_all.bat [arches_csv] [lib_types_csv] [build_types_csv] [crt_csv]`: 循环每个逗号分隔的组合（默认为 `x64`、`Static,Shared`、`Release,Debug`、`MD,MT`），并在首次失败时停止。
- `build_android.sh [Release|Debug] [Static|Shared] [ABI] [API_LEVEL]`: 接受 `arm64-v8a`、`armeabi-v7a`、`x86`、`x86_64`；API 默认为 31，但会被限制为 ABI 特定的最小值（32 位为 19，64 位为 21）。脚本现在调用 `cmake --build ... --target CYLogger`，因此 CYCoroutine 产物会自动构建。
- `build_android_all.sh`: 使用提供的构建类型/类型/API 级别为每个 ABI 迭代 `build_android.sh`。
- `build_all.sh [Release|Debug] [Static|Shared]`: 便利包装器，根据 `uname` 路由到 `build_macos.sh`、`build_linux.sh` 或 `build_windows.bat`。

## 手动 CMake 构建

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

如果绕过脚本时需要特定类型，请设置 `-DBUILD_SHARED_LIBS=ON` 或 `-DTARGET_ARCH=arm64`。

## 输出布局

```
Bin/
├── Android/
│   ├── arm64-v8a/
│   ├── armeabi-v7a/
│   ├── x86/
│   └── x86_64/
├── iOS/
│   ├── arm64/
│   ├── x86_64/
│   └── universal/
├── macOS/
│   ├── arm64/
│   ├── x86_64/
│   └── universal/
├── Linux/
│   ├── x86/
│   └── x86_64/
└── Windows/
    └── x64/
        ├── MD/
        │   ├── Debug/
        │   └── Release/
        └── MT/
            ├── Debug/
            └── Release/
```

每个架构文件夹（在 Windows 上，还有运行时子文件夹）包含 `Debug/` 和 `Release/`，当 `--shared-kinds` 包含共享构建时，同时输出 `.a` 和 `.dylib/.so/.dll`。

## 日志记录示例

```cpp
#include "ICYLogger.hpp"

int main() {
    CYLogger::GetInstance().Initialize("MyApp", CYLogger::LogLevel::Info);
    
    auto fileAppender = std::make_shared<CYLogger::CYLoggerFileAppender>("log.txt");
    auto consoleAppender = std::make_shared<CYLogger::CYLoggerConsoleAppender>();

    CYLogger::GetInstance().AddAppender(fileAppender);
    CYLogger::GetInstance().AddAppender(consoleAppender);
    
    CY_LOG_INFO("Application started");
    CY_LOG_WARN("This is a warning message");
    CY_LOG_ERROR("This is an error message");
    
    CYLogger::GetInstance().Shutdown();
    return 0;
}
```

## 许可证

CYLogger 在 MIT 许可证下分发。有关详细信息，请参阅 `LICENSE`。

## 贡献

欢迎提交问题和拉取请求。报告构建问题时，请描述目标平台、编译器和相关脚本参数。

## 更新日志

有关更新的时间顺序列表，请参阅 [`Change.log`](Change.log)。

