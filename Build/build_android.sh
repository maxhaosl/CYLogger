#!/bin/bash

# 构建脚本 for Android
set -e

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
ANDROID_ABI=${3:-arm64-v8a}
ANDROID_API_LEVEL=${4:-21}

echo "Building CYLogger for Android..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Android ABI: $ANDROID_ABI"
echo "Android API Level: $ANDROID_API_LEVEL"

# 检查Android NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME environment variable is not set!"
    echo "Please set ANDROID_NDK_HOME to your Android NDK path."
    exit 1
fi

# 创建构建目录
if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_android_static_${ANDROID_ABI}_$BUILD_TYPE"
else
    BUILD_SUBDIR="build_android_shared_${ANDROID_ABI}_$BUILD_TYPE"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"

cd "$BUILD_PATH"

# 配置CMake
echo "Configuring CMake..."
if [ "$LIB_TYPE" = "Static" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
          -DANDROID_ABI="$ANDROID_ABI" \
          -DANDROID_NATIVE_API_LEVEL="$ANDROID_API_LEVEL" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=OFF \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
else
    cmake -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
          -DANDROID_ABI="$ANDROID_ABI" \
          -DANDROID_NATIVE_API_LEVEL="$ANDROID_API_LEVEL" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=ON \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
fi

# 编译
echo "Building..."
if command -v nproc >/dev/null 2>&1; then
    make -j$(nproc)
else
    # macOS上使用sysctl获取CPU核心数
    make -j$(sysctl -n hw.ncpu)
fi

echo "Build completed successfully!"
echo "Output directory: $PROJECT_ROOT/Bin/Android/$ANDROID_ABI/$LIB_TYPE"

cd "$BUILD_DIR"