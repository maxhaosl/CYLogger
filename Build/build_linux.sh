#!/bin/bash

# Linux build helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
RAW_ARCH=${3:-$(uname -m)}

# Normalize architecture name
case "$RAW_ARCH" in
    x86_64|amd64|AMD64)
        TARGET_ARCH="x86_64"
        ;;
    i386|i686|x86)
        TARGET_ARCH="x86"
        ;;
    arm64|aarch64)
        TARGET_ARCH="arm64"
        ;;
    *)
        TARGET_ARCH="$RAW_ARCH"
        ;;
esac

detect_jobs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc && return
    fi
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 4
}

echo "Building CYLogger for Linux..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Target Architecture: $TARGET_ARCH"

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_linux_static_${TARGET_ARCH}_$BUILD_TYPE"
    BUILD_SHARED="OFF"
else
    BUILD_SUBDIR="build_linux_shared_${TARGET_ARCH}_$BUILD_TYPE"
    BUILD_SHARED="ON"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"
cd "$BUILD_PATH"

echo "Configuring CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DBUILD_SHARED_LIBS="$BUILD_SHARED"
    -DBUILD_EXAMPLES=ON
    -DTARGET_ARCH="$TARGET_ARCH"
    -DCMAKE_SYSTEM_PROCESSOR_OVERRIDE="$TARGET_ARCH"
)

# Add architecture-specific compiler flags
if [ "$TARGET_ARCH" = "x86" ]; then
    CMAKE_ARGS+=(-DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_EXE_LINKER_FLAGS=-m32)
elif [ "$TARGET_ARCH" = "x86_64" ]; then
    CMAKE_ARGS+=(-DCMAKE_C_FLAGS=-m64 -DCMAKE_CXX_FLAGS=-m64)
fi

cmake "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"

echo "Building..."
cmake --build . --parallel "$(detect_jobs)"

OUTPUT_DIR="$PROJECT_ROOT/Bin/Linux/$TARGET_ARCH/$BUILD_TYPE"
echo "Build completed successfully!"
echo "Output directory: $OUTPUT_DIR"

if [ -f "$OUTPUT_DIR/CYLoggerConsoleTest" ]; then
    echo ""
    echo "Running console test..."
    "$OUTPUT_DIR/CYLoggerConsoleTest"
fi

cd "$BUILD_DIR"