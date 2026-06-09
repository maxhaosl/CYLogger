#!/bin/bash

# ============================================================
# CYLogger Android Build Script
# ============================================================
# Builds CYLogger and its dependencies for a single Android ABI + build type.
# Calls standalone CYCommon and CYCoroutine build scripts for deps.
# Output: CYLogger/Bin/Android/<ABI>/<BuildType>/
#
# Usage: build_android.sh [BuildType] [LibType] [ABI] [APILevel]
#   BuildType: Debug (default) or Release
#   LibType: Static (default) or Shared
#   ABI: arm64-v8a (default), armeabi-v7a, x86, x86_64
#   APILevel: Android API level (default: 31)
# ============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$(dirname "$SCRIPT_DIR")"
CYCOMMON_ROOT="$SOURCE_DIR/ThirdParty/CYCommon"
CYCOROUTINE_ROOT="$SOURCE_DIR/ThirdParty/CYCoroutine"
BUILD_TYPE=${1:-Release}
ANDROID_ABI=${2:-arm64-v8a}
ANDROID_API_LEVEL=${3:-31}

# Build script paths
CYCOMMON_BUILD_SCRIPT="$CYCOMMON_ROOT/Build/build_android.sh"
CYCOROUTINE_BUILD_SCRIPT="$CYCOROUTINE_ROOT/Build/build_android.sh"

detect_android_sdk() {
    local -a candidates=(
        "${ANDROID_SDK_ROOT:-}"
        "${ANDROID_HOME:-}"
        "${HOME}/Library/Android/sdk"
        "${HOME}/Android/Sdk"
        "${HOME}/Android/sdk"
        "/usr/local/share/android-sdk"
        "/opt/android-sdk"
    )
    for path in "${candidates[@]}"; do
        if [ -n "$path" ] && [ -d "$path" ]; then
            echo "$path"
            return
        fi
    done
}

detect_android_ndk() {
    local sdk_root=$1
    local -a candidates=(
        "${ANDROID_NDK_HOME:-}"
        "${ANDROID_NDK_ROOT:-}"
        "${ANDROID_NDK:-}"
    )
    for path in "${candidates[@]}"; do
        if [ -n "$path" ] && [ -d "$path" ] && [ -f "$path/build/cmake/android.toolchain.cmake" ]; then
            echo "$path"
            return
        fi
    done

    if [ -d "$sdk_root/ndk" ]; then
        local candidate
        for candidate in $(ls -1 "$sdk_root/ndk" | sort -r); do
            local full_path="$sdk_root/ndk/$candidate"
            if [ -d "$full_path" ] && [ -f "$full_path/build/cmake/android.toolchain.cmake" ]; then
                echo "$full_path"
                return
            fi
        done
    fi

    if [ -d "$sdk_root/ndk-bundle" ] && [ -f "$sdk_root/ndk-bundle/build/cmake/android.toolchain.cmake" ]; then
        echo "$sdk_root/ndk-bundle"
    fi
}

android_api_for_abi() {
    local abi=$1
    local requested=$2
    local min_api
    case "$abi" in
        armeabi-v7a|x86)
            min_api=19
            ;;
        *)
            min_api=21
            ;;
    esac

    if ! [[ "$requested" =~ ^[0-9]+$ ]]; then
        requested=$min_api
    fi

    if [ "$requested" -lt "$min_api" ]; then
        echo "$min_api"
    else
        echo "$requested"
    fi
}

log() {
    echo "[CYLogger build] $1"
}

# Detect SDK/NDK
SDK_ROOT=$(detect_android_sdk)
if [ -z "$SDK_ROOT" ]; then
    log "Error: Android SDK not found. Install it or set ANDROID_SDK_ROOT."
    exit 1
fi

NDK_HOME=$(detect_android_ndk "$SDK_ROOT")
if [ -z "$NDK_HOME" ]; then
    log "Error: Android NDK not found under $SDK_ROOT. Install it via sdkmanager."
    exit 1
fi

export ANDROID_SDK_ROOT="$SDK_ROOT"
export ANDROID_HOME="$SDK_ROOT"
export ANDROID_NDK_HOME="$NDK_HOME"
export ANDROID_NDK_ROOT="$NDK_HOME"

ANDROID_API_LEVEL=$(android_api_for_abi "$ANDROID_ABI" "$ANDROID_API_LEVEL")

if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi

log "CYLogger Android Build"
log "  Build Type: $BUILD_TYPE"
log "  ABI: $ANDROID_ABI"
log "  API Level: $ANDROID_API_LEVEL"
log "  SDK: $SDK_ROOT"
log "  NDK: $NDK_HOME"

# ============================================================
# Step 1: Build CYCommon (standalone)
# ============================================================
# CYCommon build_android.sh expects: BUILD_TYPE LIB_TYPE ABI API_LEVEL
log "Building CYCommon for $ANDROID_ABI/$BUILD_TYPE..."
if ! "$CYCOMMON_BUILD_SCRIPT" "$BUILD_TYPE" "static" "$ANDROID_ABI" "$ANDROID_API_LEVEL"; then
    log "Error: CYCommon build failed."
    exit 1
fi

# ============================================================
# Step 2: Build CYCoroutine (standalone, depends on CYCommon)
# ============================================================
log "Building CYCoroutine for $ANDROID_ABI/$BUILD_TYPE..."
if ! "$CYCOROUTINE_BUILD_SCRIPT" "$BUILD_TYPE" "$ANDROID_ABI" "$ANDROID_API_LEVEL"; then
    log "Error: CYCoroutine build failed."
    exit 1
fi

# ============================================================
# Step 3: Build CYLogger itself
# ============================================================
log "Building CYLogger for $ANDROID_ABI/$BUILD_TYPE..."

# Debug suffix
if [ "$BUILD_TYPE" = "Debug" ]; then
    _debug_suffix="D"
else
    _debug_suffix=""
fi

# --- Static build: build CYLoggerStatic + fmt (separate dir) ---
log "Static build..."
STATIC_BUILD_PATH="$SCRIPT_DIR/build_android_${ANDROID_ABI}_${BUILD_TYPE}_static"
mkdir -p "$STATIC_BUILD_PATH"
log "Configuring CMake (static)..."
cmake -S "$SOURCE_DIR" -B "$STATIC_BUILD_PATH" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ANDROID_ABI" \
    -DANDROID_PLATFORM="android-$ANDROID_API_LEVEL" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DTARGET_ARCH="$ANDROID_ABI" \
    -DANDROID_SDK_ROOT="$SDK_ROOT" \
    -DANDROID_NDK="$NDK_HOME" \
    -DANDROID_STL=c++_static \
    -DCYLOGGER_SKIP_SHARED=ON

log "Building CYLoggerStatic and fmt..."
cmake --build "$STATIC_BUILD_PATH" --target CYLoggerStatic fmt --parallel "$JOBS"

# --- Shared build: configure + build (separate dir) ---
log "Shared build..."
SHARED_BUILD_PATH="$SCRIPT_DIR/build_android_${ANDROID_ABI}_${BUILD_TYPE}_shared"
mkdir -p "$SHARED_BUILD_PATH"
log "Configuring CMake (shared)..."
cmake -S "$SOURCE_DIR" -B "$SHARED_BUILD_PATH" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ANDROID_ABI" \
    -DANDROID_PLATFORM="android-$ANDROID_API_LEVEL" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_STATIC_LIBS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DTARGET_ARCH="$ANDROID_ABI" \
    -DANDROID_SDK_ROOT="$SDK_ROOT" \
    -DANDROID_NDK="$NDK_HOME" \
    -DANDROID_STL=c++_static \
    -DCYLOGGER_SKIP_SHARED=OFF

log "Building CYLogger_shared..."
cmake --build "$SHARED_BUILD_PATH" --target CYLogger_shared --parallel "$JOBS"

# ============================================================
# Step 4: Copy artifacts to expected output directory
# ============================================================
EXPECTED_OUTPUT="$SOURCE_DIR/Bin/Android/$ANDROID_ABI/$BUILD_TYPE"
mkdir -p "$EXPECTED_OUTPUT"

log "Copying artifacts to $EXPECTED_OUTPUT..."

# CYCommon
if [ -f "$CYCOMMON_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCommon${_debug_suffix}.a" ]; then
    cp "$CYCOMMON_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCommon${_debug_suffix}.a" "$EXPECTED_OUTPUT/"
    log "  Placed: libCYCommon${_debug_suffix}.a"
fi

# CYCoroutine
if [ -f "$CYCOROUTINE_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCoroutine${_debug_suffix}.a" ]; then
    cp "$CYCOROUTINE_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCoroutine${_debug_suffix}.a" "$EXPECTED_OUTPUT/"
    log "  Placed: libCYCoroutine${_debug_suffix}.a"
fi
if [ -f "$CYCOROUTINE_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCoroutine${_debug_suffix}.so" ]; then
    cp "$CYCOROUTINE_ROOT/Bin/Android/$ANDROID_ABI/$BUILD_TYPE/libCYCoroutine${_debug_suffix}.so" "$EXPECTED_OUTPUT/"
    log "  Placed: libCYCoroutine${_debug_suffix}.so"
fi

# fmt (from static build output dir, fmt follows CYLogger's output directory)
if [ "$BUILD_TYPE" = "Debug" ]; then
    _fmt_lib="libfmtD.a"
else
    _fmt_lib="libfmt.a"
fi
if [ -f "$EXPECTED_OUTPUT/$_fmt_lib" ]; then
    cp "$EXPECTED_OUTPUT/$_fmt_lib" "$EXPECTED_OUTPUT/" 2>/dev/null || true
    log "  Placed: $_fmt_lib"
fi

# CYLogger static (from static build output dir)
if [ -f "$EXPECTED_OUTPUT/libCYLogger${_debug_suffix}.a" ]; then
    log "  OK: libCYLogger${_debug_suffix}.a (already in place)"
fi

# CYLogger shared (from shared build output dir)
if [ -f "$EXPECTED_OUTPUT/libCYLogger${_debug_suffix}.so" ]; then
    log "  OK: libCYLogger${_debug_suffix}.so (already in place)"
fi

log "Build completed successfully!"
log "Output directory: $EXPECTED_OUTPUT"
