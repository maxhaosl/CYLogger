#!/bin/bash

# 构建脚本 for macOS
set -e

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}

echo "Building CYLogger for macOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"

# 创建构建目录
if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_macos_static_$BUILD_TYPE"
else
    BUILD_SUBDIR="build_macos_shared_$BUILD_TYPE"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"

cd "$BUILD_PATH"

# 配置CMake
echo "Configuring CMake..."
if [ "$LIB_TYPE" = "Static" ]; then
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=OFF \
          -DBUILD_EXAMPLES=ON \
          ${CMAKE_OSX_ARCHITECTURES:+-DCMAKE_OSX_ARCHITECTURES=$CMAKE_OSX_ARCHITECTURES} \
          "$PROJECT_ROOT"
else
    cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=ON \
          -DBUILD_EXAMPLES=ON \
          ${CMAKE_OSX_ARCHITECTURES:+-DCMAKE_OSX_ARCHITECTURES=$CMAKE_OSX_ARCHITECTURES} \
          "$PROJECT_ROOT"
fi

# 编译
echo "Building..."
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"
echo "Output directory: $PROJECT_ROOT/Bin/macOS/x64/$BUILD_TYPE"

# 运行测试
if [ -f "$PROJECT_ROOT/Bin/macOS/x64/$BUILD_TYPE/CYLoggerConsoleTest" ]; then
    echo ""
    echo "Running console test..."
    "$PROJECT_ROOT/Bin/macOS/x64/$BUILD_TYPE/CYLoggerConsoleTest"
fi

cd "$BUILD_DIR"