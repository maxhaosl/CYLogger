#!/bin/bash

# iOS build helper script
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
REQUESTED_ARCH=${3:-arm64}

case "$REQUESTED_ARCH" in
    arm64)
        IOS_PLATFORM="OS64"
        CMAKE_ARCH="arm64"
        OUTPUT_ARCH="arm64"
        ;;
    x86_64)
        IOS_PLATFORM="SIMULATOR64"
        CMAKE_ARCH="x86_64"
        OUTPUT_ARCH="x86_64"
        ;;
    arm64-simulator)
        IOS_PLATFORM="SIMULATORARM64"
        CMAKE_ARCH="arm64"
        OUTPUT_ARCH="arm64-simulator"
        ;;
    *)
        echo "Unsupported iOS architecture: $REQUESTED_ARCH"
        exit 1
        ;;
esac

IOS_TOOLCHAIN="$PROJECT_ROOT/cmake/ios.toolchain.cmake"
if [ ! -f "$IOS_TOOLCHAIN" ]; then
    echo "Cannot find iOS toolchain file at $IOS_TOOLCHAIN"
    exit 1
else
    echo "Using iOS toolchain: $IOS_TOOLCHAIN"
fi

echo "Building CYLogger for iOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "iOS Architecture: $OUTPUT_ARCH"
echo "iOS Platform: $IOS_PLATFORM"

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_ios_static_${OUTPUT_ARCH}_$BUILD_TYPE"
else
    BUILD_SUBDIR="build_ios_shared_${OUTPUT_ARCH}_$BUILD_TYPE"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"
cd "$BUILD_PATH"

echo "Configuring CMake..."
if [ "$LIB_TYPE" = "Static" ]; then
    cmake -DCMAKE_TOOLCHAIN_FILE="$IOS_TOOLCHAIN" \
          -DPLATFORM="$IOS_PLATFORM" \
          -DARCHS="$CMAKE_ARCH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=OFF \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
else
    cmake -DCMAKE_TOOLCHAIN_FILE="$IOS_TOOLCHAIN" \
          -DPLATFORM="$IOS_PLATFORM" \
          -DARCHS="$CMAKE_ARCH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=ON \
          -DBUILD_EXAMPLES=OFF \
          "$PROJECT_ROOT"
fi

# Ensure CYCoroutine dependencies are built first
ensure_cycoroutine_ios() {
    local arch_dir="$PROJECT_ROOT/Bin/iOS/$OUTPUT_ARCH/$BUILD_TYPE"
    local static_lib="$arch_dir/libCYCoroutine.a"
    local shared_lib="$arch_dir/libCYCoroutine.dylib"

    local need_static=0
    local need_shared=0

    if [ ! -f "$static_lib" ]; then
        need_static=1
    fi

    if [ "$LIB_TYPE" = "Shared" ] && [ ! -f "$shared_lib" ]; then
        need_shared=1
    fi

    if [ "$need_static" -eq 0 ] && [ "$need_shared" -eq 0 ]; then
        echo "CYCoroutine dependencies already present for ${OUTPUT_ARCH}/${BUILD_TYPE}/${LIB_TYPE}"
        return
    fi

    local shared_flag
    if [ "$LIB_TYPE" = "Shared" ]; then
        shared_flag="ON"
    else
        shared_flag="OFF"
    fi

    local dep_dir="$BUILD_DIR/deps_ios_${OUTPUT_ARCH}_${BUILD_TYPE}_${shared_flag}"
    local -a dep_cmake_args=(
        "-DCMAKE_TOOLCHAIN_FILE=$IOS_TOOLCHAIN"
        "-DPLATFORM=$IOS_PLATFORM"
        "-DARCHS=$CMAKE_ARCH"
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=$shared_flag"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
    )

    echo "Preparing CYCoroutine dependency for ${OUTPUT_ARCH}/${BUILD_TYPE}/${LIB_TYPE}..."
    cmake -S "$PROJECT_ROOT" -B "$dep_dir" "${dep_cmake_args[@]}"

    if [ "$need_static" -eq 1 ]; then
        echo "Building CYCoroutine_static for ${OUTPUT_ARCH}/${BUILD_TYPE}..."
        cmake --build "$dep_dir" --target CYCoroutine_static --parallel "$(sysctl -n hw.ncpu)"
    fi

    if [ "$need_shared" -eq 1 ]; then
        echo "Building CYCoroutine_shared for ${OUTPUT_ARCH}/${BUILD_TYPE}..."
        cmake --build "$dep_dir" --target CYCoroutine_shared --parallel "$(sysctl -n hw.ncpu)"
    fi
}

ensure_cycoroutine_ios

echo "Building..."
cmake --build . --parallel "$(sysctl -n hw.ncpu)"

echo "Build completed successfully!"
echo "Output directory: $PROJECT_ROOT/Bin/iOS/$OUTPUT_ARCH/$BUILD_TYPE"

cd "$BUILD_DIR"