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

ensure_cycommon_submodule() {
    # CYCommon can be in multiple locations:
    # 1. ThirdParty/CYCommon (bundled with CYLogger)
    # 2. ../../../CYCommon (sibling location)
    local cycommon_header="$PROJECT_ROOT/ThirdParty/CYCommon/Inc/CYCommon/CYCommon.hpp"
    local cycommon_build="$PROJECT_ROOT/ThirdParty/CYCommon/Build/CMakeLists.txt"

    if [ -f "$cycommon_header" ] && [ -f "$cycommon_build" ]; then
        return 0
    fi

    # Check sibling location
    local cycommon_sibling="$PROJECT_ROOT/../../../CYCommon/Inc/CYCommon/CYCommon.hpp"
    if [ -f "$cycommon_sibling" ]; then
        return 0
    fi

    echo "CYCommon not found at expected locations:"
    echo "  - $cycommon_header"
    echo "  - $cycommon_sibling"
    echo "Please ensure CYCommon is available as a local dependency."
    return 1
}

ensure_fmt_submodule
ensure_cycommon_submodule || exit 1

# Ensure CYCommon static lib exists in CYLogger/Bin/Linux/<arch>/<config>/
build_cycommon_for_slice() {
    "$BUILD_DIR/build_cycommon_linux.sh" "$BUILD_TYPE" "$TARGET_ARCH"
}

# TARGET_ARCH is set later; call after canonicalization below

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

# Verify that a C++ toolchain can compile and link (clang may lack libstdc++ on some Linux images).
linux_cxx_toolchain_works() {
    local cxx="$1" cc="$2" tmpdir probe
    [ -z "$cxx" ] && return 1
    tmpdir=$(mktemp -d)
    probe="$tmpdir/probe.cxx"
    printf 'int main(){return 0;}\n' >"$probe"
    if [ -n "$cc" ]; then
        "$cxx" "$probe" -o "$tmpdir/probe" -std=c++20 2>/dev/null && [ -x "$tmpdir/probe" ]
    else
        "$cxx" "$probe" -o "$tmpdir/probe" -std=c++20 2>/dev/null && [ -x "$tmpdir/probe" ]
    fi
    local ok=$?
    rm -rf "$tmpdir"
    return "$ok"
}

# Find best available compiler (prefer clang-17, fall back to clang, then gcc)
find_best_cc() {
    local cc
    cc=$(detect_compiler "${CYLOGGER_CC:-}" "clang-17");   [ -n "$cc" ] && echo "$cc" && return
    cc=$(detect_compiler "${CYLOGGER_CC:-}" "clang");      [ -n "$cc" ] && echo "$cc" && return
    cc=$(detect_compiler "${CYLOGGER_CC:-}" "gcc");        [ -n "$cc" ] && echo "$cc" && return
    echo ""
}

find_best_cxx() {
    local cxx
    cxx=$(detect_compiler "${CYLOGGER_CXX:-}" "clang++-17"); [ -n "$cxx" ] && echo "$cxx" && return
    cxx=$(detect_compiler "${CYLOGGER_CXX:-}" "clang++");    [ -n "$cxx" ] && echo "$cxx" && return
    cxx=$(detect_compiler "${CYLOGGER_CXX:-}" "g++");        [ -n "$cxx" ] && echo "$cxx" && return
    echo ""
}

select_linux_toolchain() {
    local cc cxx
    if [ -n "${CYLOGGER_CC:-}" ] && [ -n "${CYLOGGER_CXX:-}" ]; then
        echo "${CYLOGGER_CC}|${CYLOGGER_CXX}"
        return
    fi
    cc=$(find_best_cc)
    cxx=$(find_best_cxx)
    if linux_cxx_toolchain_works "$cxx" "$cc"; then
        echo "${cc}|${cxx}"
        return
    fi
    cc=$(detect_compiler "" "gcc")
    cxx=$(detect_compiler "" "g++")
    if linux_cxx_toolchain_works "$cxx" "$cc"; then
        echo "${cc}|${cxx}"
        return
    fi
    echo "${cc}|${cxx}"
}

IFS='|' read -r CC_BIN CXX_BIN <<<"$(select_linux_toolchain)"

if [ -z "$CC_BIN" ] || [ -z "$CXX_BIN" ]; then
    echo "Error: No suitable C/C++ compiler found." >&2
    echo "Tried: clang-17, clang, gcc (or set CYLOGGER_CC/CYLOGGER_CXX to override)" >&2
    exit 1
fi

CC_NAME=$(basename "$CC_BIN")
CXX_NAME=$(basename "$CXX_BIN")
if [ "$CC_NAME" = "clang-17" ] && [ "$CXX_NAME" = "clang++-17" ]; then
    : # clang-17 found, no note needed
elif command -v clang-17 >/dev/null 2>&1 && command -v clang++-17 >/dev/null 2>&1; then
    echo "Note: clang-17 is installed but not in use (using $CC_NAME / $CXX_NAME)"
else
    echo "Note: Using $CC_NAME / $CXX_NAME (clang-17 not available)"
fi

export CYLOGGER_CC="$CC_BIN"
export CYLOGGER_CXX="$CXX_BIN"

if ! build_cycommon_for_slice; then
    echo "Error: CYCommon build failed." >&2
    exit 1
fi

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
    CYLOGGER_WANT_SHARED="OFF"
else
    BUILD_SUBDIR="build_linux_shared_${TARGET_ARCH}_$BUILD_TYPE"
    BUILD_SHARED="ON"
    CYLOGGER_WANT_SHARED="ON"
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
    -DCYLOGGER_WANT_SHARED="$CYLOGGER_WANT_SHARED"
    -DBUILD_EXAMPLES=OFF
    -DTARGET_ARCH="$TARGET_ARCH"
    -DCMAKE_SYSTEM_PROCESSOR_OVERRIDE="$TARGET_ARCH"
)

# Add architecture-specific compiler flags
if [ "$TARGET_ARCH" = "x86_64" ]; then
    CMAKE_ARGS+=(-DCMAKE_C_FLAGS=-m64)
fi

# If using clang and libc++ is available on this system, use it.
# Otherwise clang defaults to libstdc++ which is the right choice.
if [[ "$CXX_NAME" == clang* ]]; then
    if [ -f /usr/lib/x86_64-linux-gnu/libc++.so ] || [ -f /usr/lib/libc++.so ]; then
        CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="-stdlib=libc++")
    fi
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
