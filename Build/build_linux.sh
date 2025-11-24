#!/bin/bash

# Linux build helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
LINUX_PLATFORM_KEY="linux"
LINUX_PLATFORM_DIR="$(map_platform_dir "$LINUX_PLATFORM_KEY")"
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
RAW_ARCH=${3:-$(uname -m)}

ensure_fmt_submodule() {
    local fmt_header="$PROJECT_ROOT/ThirdParty/fmt/include/fmt/format.h"
    if [ -f "$fmt_header" ]; then
        return 0
    fi

    echo "fmt headers not found. Initializing ThirdParty/fmt submodule..."
    if git -C "$PROJECT_ROOT" submodule update --init --recursive ThirdParty/fmt; then
        if [ -f "$fmt_header" ]; then
            echo "fmt submodule ready."
            return 0
        fi
        echo "fmt headers still missing after submodule update."
        exit 1
    else
        echo "Failed to update fmt submodule. Ensure git is installed and accessible."
        exit 1
    fi
}

ensure_fmt_submodule

canonicalize_linux_arch() {
    local token
    token=$(printf '%s' "$1" | xargs)
    local lower
    lower=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
    case "$lower" in
        x86_64|amd64|x64)
            echo "x86_64"
            ;;
        i386|i686|x86)
            echo "x86"
            ;;
        arm64|aarch64)
            echo "arm64"
            ;;
        *)
            echo "$token"
            ;;
    esac
}

REQUESTED_ARCH=$(printf '%s' "$RAW_ARCH" | xargs)
TARGET_ARCH=$(canonicalize_linux_arch "$REQUESTED_ARCH")

detect_jobs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc && return
    fi
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 4
}

detect_compiler() {
    local override="$1" required="$2" resolved
    if [ -n "$override" ]; then
        echo "$override"
        return
    fi
    if command -v "$required" >/dev/null 2>&1; then
        resolved=$(command -v "$required")
        echo "$resolved"
        return
    fi
    echo ""
}

CC_BIN=$(detect_compiler "${CYLOGGER_CC:-}" "clang-17")
CXX_BIN=$(detect_compiler "${CYLOGGER_CXX:-}" "clang++-17")

if [ -z "$CC_BIN" ] || [ -z "$CXX_BIN" ]; then
    echo "Error: clang-17 toolchain not found (set CYLOGGER_CC/CYLOGGER_CXX to override)." >&2
    exit 1
fi

export CYLOGGER_CC="$CC_BIN"
export CYLOGGER_CXX="$CXX_BIN"

echo "Building CYLogger for Linux..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
if [ "$REQUESTED_ARCH" != "$TARGET_ARCH" ]; then
    echo "Target Architecture: $TARGET_ARCH (requested: $REQUESTED_ARCH)"
else
    echo "Target Architecture: $TARGET_ARCH"
fi

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
    "-DCMAKE_C_COMPILER=$CC_BIN"
    "-DCMAKE_CXX_COMPILER=$CXX_BIN"
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

OUTPUT_DIR="$(platform_slice_dir "$LINUX_PLATFORM_KEY" "$TARGET_ARCH" "$BUILD_TYPE")"
echo "Build completed successfully!"
echo "Output directory: $OUTPUT_DIR"

if [ -f "$OUTPUT_DIR/CYLoggerConsoleTest" ]; then
    echo ""
    echo "Running console test..."
    "$OUTPUT_DIR/CYLoggerConsoleTest"
fi

cd "$BUILD_DIR"
