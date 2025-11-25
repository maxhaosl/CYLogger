[English](README.md) | [中文](README_zh.md)

# CYLogger

CYLogger is a cross-platform logging library powered by C++20 coroutines. It offers coroutine-aware appenders, lock-free buffers, flexible formatting, and portable build scripts that ship both static and shared libraries for every supported platform.

## Features

- C++20 coroutine-based log pipeline with CYCoroutine integration
- Lock-isolated, double-buffer queue for deterministic throughput
- Rich pattern/layout/filter system with template-based customization
- Multiple appenders: console, file, remote, system log, zip rotation, etc.
- Built-in exception handling, statistics, and FPS counters for profiling
- Unified binary layout under `Bin/<Platform>/<Arch>[/<CRT>]/<Config>`

## Supported Platforms

- Windows: x64 (Visual Studio or MinGW)
- macOS: arm64, x86_64, and universal slices (static & dynamic)
- iOS: arm64, x86_64 simulator, arm64 simulator, plus universal slices
- Linux: x86_64, x86 (requires multilib when targeting 32-bit)
- Android: armeabi-v7a, arm64-v8a, x86, x86_64

## Dependencies

- [CYCoroutine](ThirdParty/CYCoroutine) – bundled and built automatically. On Windows, only static libraries (`CYCoroutine.lib`) are produced; on other platforms, both static and shared libraries are available. Its macOS/iOS helpers now emit universal slices under `ThirdParty/CYCoroutine/Bin/<platform>/universal/<config>`, so CYLogger no longer needs a separate `lipo` pass when prepping dependencies.
- CMake 3.16+
- C++20-compatible toolchains (MSVC 19.3x, Clang 14+, GCC 11+)
- Linux builds default to `clang-17`/`clang++-17`; override with `CYLOGGER_CC` / `CYLOGGER_CXX` if you need a different compiler.
- Android NDK r26b+ (auto-detected from common SDK folders)

## Build Requirements

- macOS/iOS: Xcode command-line tools, `lipo`
- Linux: `build-essential`, `cmake`, optional `gcc-multilib` for `x86`
- Android: Android SDK + NDK (auto-detected, no manual path export required)
- Windows: Visual Studio 2019/2022 with CMake, or MSYS2/MinGW shell

## Build Scripts Overview (`Build/`)

| Script | Description | Arguments |
| --- | --- | --- |
| `build_all_platforms.sh` | Matrix builder that stitches every platform/arch/config, generates universal Apple binaries, auto-builds CYCoroutine if missing. | `--platforms`, `--build-types`, `--shared-kinds`, `--linux-arches`, `--android-abis`, `--android-api-level`, `--force-linux` |
| `build_all.sh` | Host-aware wrapper that delegates to the correct single-platform helper based on `uname`. | `[build_type] [Static|Shared]` |
| `build_macos.sh` | Builds macOS static/shared slices (arm64/x86_64). | `[build_type] [Static|Shared]` |
| `build_ios.sh` | Builds iOS slices (device, simulator, simulator-arm64). | `[build_type] [Static|Shared] [arch]` |
| `build_linux.sh` | Builds Linux static/shared archives for the requested arch. | `[build_type] [Static|Shared] [arch]` |
| `build_linux_all.sh` | Loops `build_linux.sh` over all arches, build types, lib kinds. | `[build_types_csv] [lib_types_csv] [arches_csv]` |
| `build_windows.bat` | Builds Windows libraries for the selected architecture/runtime. | `[build_type] [Static|Shared] [arch] [MD|MT]` |
| `build_windows_all.bat` | Matrix builder for every Windows arch/build/lib/CRT combo. | `[arches_csv] [lib_types_csv] [build_types_csv] [crt_csv]` |
| `build_android.sh` | Builds one Android ABI. Detects SDK/NDK automatically. | `[build_type] [Static|Shared] [abi] [api_level]` |
| `build_android_all.sh` | Convenience wrapper that loops over all Android ABIs. | `[build_type] [Static|Shared] [api_level]` |
| `list_android_libs.sh` | Lists all Android artifacts that were produced. | _none_ |

All scripts exit on the first failure (`set -euo pipefail`) and surface missing dependency errors early.

## `build_all_platforms.sh`

This is the primary entry point when you need every platform output in `Bin/`. Key behaviors:

- Automatically checks that CYCoroutine artifacts exist for each slice; missing ones trigger an inline `cmake --build ... --target CYCoroutine_{static,shared}` before building CYLogger.
- Auto-detects `ANDROID_SDK_ROOT` / `ANDROID_NDK_HOME` from the standard locations (`~/Library/Android/sdk`, `$ANDROID_HOME`, `/usr/local/share/android-sdk`, etc.).
- Generates macOS/iOS universal libraries (`lipo`), covering both static (`.a`) and shared (`.dylib`) artifacts.
- Supports selective execution through CLI flags and environment overrides.

### CLI Flags

```
./Build/build_all_platforms.sh [options]

--platforms macos,ios,linux,android,windows   Limit the matrices that run (default: all).
--build-types Release,Debug                   CMake build types (default: Release,Debug).
--shared-kinds static,shared                  Library flavors; maps to BUILD_SHARED_LIBS (default: both).
--linux-arches x86_64,x86                     Linux targets to compile.
--android-abis arm64-v8a,armeabi-v7a,...      Android ABIs to compile.
--android-api-level 34                        Default API level; ABI-specific minimums are enforced.
--force-linux                                 Allow Linux builds on non-Linux hosts when cross toolchains exist.
-h, --help                                    Print usage.
```

Environment overrides: `MACOS_DEPLOYMENT_TARGET`, `IOS_DEPLOYMENT_TARGET`, `ANDROID_API_LEVEL`, `BUILD_JOBS`.

### Usage Examples

Build everything that is supported on the current host:

```bash
./Build/build_all_platforms.sh
```

Apple-only build with Release static libraries:

```bash
./Build/build_all_platforms.sh \
  --platforms macos,ios \
  --build-types Release \
  --shared-kinds static
```

Android-only build with a reduced ABI set:

```bash
./Build/build_all_platforms.sh \
  --platforms android \
  --android-abis arm64-v8a,x86_64 \
  --build-types Release,Debug
```

Linux matrix from a Docker container (useful on macOS hosts):

```bash
docker run --rm -it \
  -v "$(pwd)":/workspace -w /workspace \
  ubuntu:22.04 \
  /bin/bash -lc "apt update && apt install -y build-essential cmake ninja-build && \
                 ./Build/build_all_platforms.sh --platforms linux"
```

## Per-platform Helpers

- `build_macos.sh [Release|Debug] [Static|Shared]`: builds one flavor using `CMAKE_OSX_ARCHITECTURES` (default arm64;x86_64) and runs the console test if available; CYCoroutine's Apple helper now contributes ready-made universal libraries in parallel.
- `build_ios.sh [Release|Debug] [Static|Shared] [arm64|x86_64|arm64-simulator]`: maps to the appropriate ios-cmake platform tokens (`OS64`, `SIMULATOR64`, `SIMULATORARM64`) and benefits from CYCoroutine's automatic universal outputs when multiple slices exist.
- `build_linux.sh [Release|Debug] [Static|Shared] [x86_64|x86]`: normalizes architecture names (aarch64→arm64, amd64→x86_64), passes `-DCMAKE_SYSTEM_PROCESSOR_OVERRIDE`, automatically adds `-m32`/`-m64` compiler flags based on target architecture, and auto-detects a `clang-17` toolchain (overridable via `CYLOGGER_CC`/`CYLOGGER_CXX`).
- `build_linux_all.sh [build_types_csv] [lib_types_csv] [arches_csv]`: iterates `build_linux.sh` across the provided comma-delimited lists (defaults to `Release,Debug`, `Static,Shared`, `x86_64,x86`). Automatically ensures CYCoroutine dependencies are built with correct output paths before building CYLogger and propagates the detected compiler pair to every dependency build to keep ABI compatibility.
- `build_windows.bat [Release|Debug] [Static|Shared] [x64|Win32|arm64] [MD|MT]`: forwards the architecture/runtime to CMake and emits artifacts under `Bin/Windows/<arch>/<crt>/<config>`.
- `build_windows_all.bat [arches_csv] [lib_types_csv] [build_types_csv] [crt_csv]`: loops over every comma-separated combination (defaults to `x64`, `Static,Shared`, `Release,Debug`, `MD,MT`) and stops at the first failure.
- `build_android.sh [Release|Debug] [Static|Shared] [ABI] [API_LEVEL]`: accepts `arm64-v8a`, `armeabi-v7a`, `x86`, `x86_64`; API defaults to 31 but is clamped to ABI-specific minimums (19 for 32-bit, 21 for 64-bit). The script now invokes `cmake --build ... --target CYLogger`, so CYCoroutine artifacts are built automatically.
- `build_android_all.sh`: iterates `build_android.sh` for every ABI with the supplied build type/flavor/API level.
- `build_all.sh [Release|Debug] [Static|Shared]`: convenience wrapper that routes to `build_macos.sh`, `build_linux.sh`, or `build_windows.bat` based on `uname`.

## Manual CMake Build

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

Set `-DBUILD_SHARED_LIBS=ON` or `-DTARGET_ARCH=arm64` if you need specific flavors when bypassing the scripts.

## Output Layout

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

Each architecture folder (and, on Windows, runtime subfolder) contains `Debug/` and `Release/`, with both `.a` and `.dylib/.so/.dll` outputs when `--shared-kinds` includes shared builds.

## Logging Example

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

## License

CYLogger is distributed under the MIT License. See `LICENSE` for details.

## Contributing

Issues and pull requests are welcome. Please describe the target platform, compiler, and relevant script arguments when reporting build problems.

## Changelog

See [`Change.log`](Change.log) for a chronological list of updates.
