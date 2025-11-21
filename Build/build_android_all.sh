#!/bin/bash

# 构建所有Android架构的库脚本
set -e

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
ANDROID_API_LEVEL=${3:-31}

echo "Building CYLogger for all Android architectures..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Android API Level: $ANDROID_API_LEVEL"

# 检查Android NDK
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "Error: ANDROID_NDK_HOME environment variable is not set!"
    echo "Please set ANDROID_NDK_HOME to your Android NDK path."
    exit 1
fi

# Android架构列表
ANDROID_ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")

# 构建每个架构
for ABI in "${ANDROID_ABIS[@]}"; do
    echo ""
    echo "========================================"
    echo "Building for Android ABI: $ABI"
    echo "========================================"
    
    "$BUILD_DIR/build_android.sh" "$BUILD_TYPE" "$LIB_TYPE" "$ABI" "$ANDROID_API_LEVEL"
    
    if [ $? -eq 0 ]; then
        echo "Successfully built for $ABI"
    else
        echo "Failed to build for $ABI"
        exit 1
    fi
done

echo ""
echo "========================================"
echo "All Android builds completed successfully!"
echo "========================================"

# 列出生成的库
echo ""
echo "Generated libraries:"
find "$PROJECT_ROOT/Bin/Android" -name "*.so" -o -name "*.a" | sort