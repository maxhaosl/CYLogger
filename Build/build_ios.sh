#!/bin/bash

# 构建脚本 for iOS
set -e

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
IOS_ARCH=${3:-arm64}

echo "Building CYLogger for iOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "iOS Architecture: $IOS_ARCH"

# 创建构建目录
if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_ios_static_${IOS_ARCH}_$BUILD_TYPE"
else
    BUILD_SUBDIR="build_ios_shared_${IOS_ARCH}_$BUILD_TYPE"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"

cd "$BUILD_PATH"

# 配置CMake for iOS
echo "Configuring CMake..."
if [ "$LIB_TYPE" = "Static" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/ios.toolchain.cmake" \
          -DPLATFORM=OS64 \
          -DARCHS="$IOS_ARCH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=OFF \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
else
    cmake -DCMAKE_TOOLCHAIN_FILE="$PROJECT_ROOT/cmake/ios.toolchain.cmake" \
          -DPLATFORM=OS64 \
          -DARCHS="$IOS_ARCH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=ON \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
fi

# 编译
echo "Building..."
make -j$(sysctl -n hw.ncpu)

echo "Build completed successfully!"
echo "Output directory: $PROJECT_ROOT/Bin/iOS/$IOS_ARCH/$BUILD_TYPE"

cd "$BUILD_DIR"