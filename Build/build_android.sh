#!/bin/bash

# Android build helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
ANDROID_ABI=${3:-arm64-v8a}
ANDROID_API_LEVEL=${4:-31}
ANDROID_PLATFORM_KEY="android"
ANDROID_PLATFORM_DIR="$(map_platform_dir "$ANDROID_PLATFORM_KEY")"
ANDROID_SLICE_DIR="$(platform_slice_dir "$ANDROID_PLATFORM_KEY" "$ANDROID_ABI" "$BUILD_TYPE")"

log() {
    echo "[build_android] $1"
}

# Validate and adjust API level based on ABI
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

    if ! [[ $requested =~ ^[0-9]+$ ]]; then
        requested=$min_api
    fi

    if [ "$requested" -lt "$min_api" ]; then
        echo "$min_api"
    else
        echo "$requested"
    fi
}

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

prepare_android_env() {
    local sdk_root
    sdk_root=$(detect_android_sdk)
    if [ -z "$sdk_root" ]; then
        echo "Error: Android SDK not found. Install it or set ANDROID_SDK_ROOT."
        exit 1
    fi

    local ndk_root
    ndk_root=$(detect_android_ndk "$sdk_root")
    if [ -z "$ndk_root" ]; then
        echo "Error: Android NDK not found under $sdk_root. Install it via sdkmanager."
        exit 1
    fi

    export ANDROID_SDK_ROOT="$sdk_root"
    export ANDROID_HOME="$sdk_root"
    export ANDROID_NDK_HOME="$ndk_root"
    export ANDROID_NDK_ROOT="$ndk_root"
    log "Using Android SDK: $ANDROID_SDK_ROOT"
    log "Using Android NDK: $ANDROID_NDK_HOME"
}

prepare_android_env

# Validate and adjust API level
ANDROID_API_LEVEL=$(android_api_for_abi "$ANDROID_ABI" "$ANDROID_API_LEVEL")

log "Building CYLogger for Android"
log "Build Type: $BUILD_TYPE"
log "Library Type: $LIB_TYPE"
log "Android ABI: $ANDROID_ABI"
log "Android API Level: $ANDROID_API_LEVEL"

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SUBDIR="build_android_static_${ANDROID_ABI}_$BUILD_TYPE"
    BUILD_SHARED="OFF"
else
    BUILD_SUBDIR="build_android_shared_${ANDROID_ABI}_$BUILD_TYPE"
    BUILD_SHARED="ON"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"
cd "$BUILD_PATH"

log "Configuring CMake..."
cmake_args=(
    "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
    "-DANDROID_ABI=$ANDROID_ABI"
    "-DANDROID_PLATFORM=android-$ANDROID_API_LEVEL"
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DBUILD_SHARED_LIBS=$BUILD_SHARED"
    "-DBUILD_EXAMPLES=OFF"
    "-DTARGET_ARCH=$ANDROID_ABI"
    "-DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
    "-DANDROID_NDK=$ANDROID_NDK_HOME"
)

cmake "${cmake_args[@]}" "$PROJECT_ROOT"

# Ensure CYCoroutine dependencies are built first
ensure_cycoroutine_android() {
    local arch_dir="$ANDROID_SLICE_DIR"
    local static_lib="$arch_dir/libCYCoroutine.a"
    local shared_lib="$arch_dir/libCYCoroutine.so"

    local need_static=0
    local need_shared=0

    if [ ! -f "$static_lib" ]; then
        need_static=1
    fi

    if [ "$LIB_TYPE" = "Shared" ] && [ ! -f "$shared_lib" ]; then
        need_shared=1
    fi

    if [ "$need_static" -eq 0 ] && [ "$need_shared" -eq 0 ]; then
        log "CYCoroutine dependencies already present for ${ANDROID_ABI}/${BUILD_TYPE}/${LIB_TYPE}"
        return
    fi

    local shared_flag
    if [ "$LIB_TYPE" = "Shared" ]; then
        shared_flag="ON"
    else
        shared_flag="OFF"
    fi

    local dep_dir="$BUILD_DIR/deps_android_${ANDROID_ABI}_${BUILD_TYPE}_${shared_flag}"
    local -a dep_cmake_args=(
        "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"
        "-DANDROID_ABI=$ANDROID_ABI"
        "-DANDROID_PLATFORM=android-$ANDROID_API_LEVEL"
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=$shared_flag"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
        "-DTARGET_ARCH=$ANDROID_ABI"
        "-DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
        "-DANDROID_NDK=$ANDROID_NDK_HOME"
    )

    log "Preparing CYCoroutine dependency for ${ANDROID_ABI}/${BUILD_TYPE}/${LIB_TYPE}..."
    cmake -S "$PROJECT_ROOT" -B "$dep_dir" "${dep_cmake_args[@]}"

    if [ "$need_static" -eq 1 ]; then
        log "Building CYCoroutine_static for ${ANDROID_ABI}/${BUILD_TYPE}..."
        if command -v nproc >/dev/null 2>&1; then
            JOBS=$(nproc)
        else
            JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
        fi
        cmake --build "$dep_dir" --target CYCoroutine_static --parallel "$JOBS"
    fi

    if [ "$need_shared" -eq 1 ]; then
        log "Building CYCoroutine_shared for ${ANDROID_ABI}/${BUILD_TYPE}..."
        if command -v nproc >/dev/null 2>&1; then
            JOBS=$(nproc)
        else
            JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
        fi
        cmake --build "$dep_dir" --target CYCoroutine_shared --parallel "$JOBS"
    fi
}

ensure_cycoroutine_android

log "Building..."
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
else
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
fi
cmake --build "$BUILD_PATH" --target CYLogger --parallel "$JOBS"

log "Build completed successfully!"
log "Output directory: $ANDROID_SLICE_DIR"

cd "$BUILD_DIR"