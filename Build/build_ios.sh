#!/bin/bash

# iOS build helper script
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"

BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
REQUESTED_ARCHES_RAW=${3:-"arm64;x86_64"}
REQUESTED_ARCHES_RAW=${REQUESTED_ARCHES_RAW//,/;}

declare -a IOS_ARCHES=()
IFS=';' read -r -a _ios_arch_tokens <<< "$REQUESTED_ARCHES_RAW"
for token in "${_ios_arch_tokens[@]}"; do
    token=$(printf '%s' "$token" | tr -d '[:space:]')
    if [ -n "$token" ]; then
        IOS_ARCHES+=("$token")
    fi
done
unset _ios_arch_tokens
if [ ${#IOS_ARCHES[@]} -eq 0 ]; then
    IOS_ARCHES+=("arm64")
fi

detect_jobs() {
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 4
}

resolve_ios_arch() {
    local arch=$1
    case "$arch" in
        arm64)
            IOS_PLATFORM_TOKEN="OS64"
            IOS_CMAKE_ARCH="arm64"
            ;;
        x86_64)
            IOS_PLATFORM_TOKEN="SIMULATOR64"
            IOS_CMAKE_ARCH="x86_64"
            ;;
        arm64-simulator)
            IOS_PLATFORM_TOKEN="SIMULATORARM64"
            IOS_CMAKE_ARCH="arm64"
            ;;
        *)
            echo "Unsupported iOS architecture: $arch"
            exit 1
            ;;
    esac
}

IOS_PLATFORM_KEY="ios"

IOS_TOOLCHAIN="$PROJECT_ROOT/cmake/ios.toolchain.cmake"
if [ ! -f "$IOS_TOOLCHAIN" ]; then
    echo "Cannot find iOS toolchain file at $IOS_TOOLCHAIN"
    exit 1
else
    echo "Using iOS toolchain: $IOS_TOOLCHAIN"
fi

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SHARED="OFF"
    BUILD_KIND_TAG="static"
else
    BUILD_SHARED="ON"
    BUILD_KIND_TAG="shared"
fi

echo "Building CYLogger for iOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Architectures: ${IOS_ARCHES[*]}"

ensure_cycoroutine_ios() {
    local arch_name=$1
    local ios_platform=$2
    local cmake_arch=$3

    local arch_dir
    arch_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch_name" "$BUILD_TYPE")"
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
        echo "CYCoroutine dependencies already present for ${arch_name}/${BUILD_TYPE}/${LIB_TYPE}"
        return
    fi

    local shared_flag="$BUILD_SHARED"
    local dep_dir="$BUILD_DIR/deps_ios_${arch_name}_${BUILD_TYPE}_${shared_flag}"
    local -a dep_cmake_args=(
        "-DCMAKE_TOOLCHAIN_FILE=$IOS_TOOLCHAIN"
        "-DPLATFORM=$ios_platform"
        "-DARCHS=$cmake_arch"
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=$shared_flag"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
        "-DTARGET_ARCH=$arch_name"
    )

    echo "Preparing CYCoroutine dependency for ${arch_name}/${BUILD_TYPE}/${LIB_TYPE}..."
    cmake -S "$PROJECT_ROOT" -B "$dep_dir" "${dep_cmake_args[@]}"

    if [ "$need_static" -eq 1 ]; then
        echo "Building CYCoroutine_static for ${arch_name}/${BUILD_TYPE}..."
        cmake --build "$dep_dir" --target CYCoroutine_static --parallel "$(detect_jobs)"
    fi

    if [ "$need_shared" -eq 1 ]; then
        echo "Building CYCoroutine_shared for ${arch_name}/${BUILD_TYPE}..."
        cmake --build "$dep_dir" --target CYCoroutine_shared --parallel "$(detect_jobs)"
    fi
}

for arch in "${IOS_ARCHES[@]}"; do
    resolve_ios_arch "$arch"
    ios_platform="$IOS_PLATFORM_TOKEN"
    cmake_arch="$IOS_CMAKE_ARCH"

    echo ""
    echo "Configuring CMake for $arch..."
    BUILD_SUBDIR="build_ios_${arch}_${BUILD_KIND_TAG}_${BUILD_TYPE}"
    BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
    mkdir -p "$BUILD_PATH"

    cmake -S "$PROJECT_ROOT" -B "$BUILD_PATH" \
          -DCMAKE_TOOLCHAIN_FILE="$IOS_TOOLCHAIN" \
          -DPLATFORM="$ios_platform" \
          -DARCHS="$cmake_arch" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS="$BUILD_SHARED" \
          -DBUILD_EXAMPLES=OFF \
          -DTARGET_ARCH="$arch"

    ensure_cycoroutine_ios "$arch" "$ios_platform" "$cmake_arch"

    echo "Building for $arch..."
    cmake --build "$BUILD_PATH" --parallel "$(detect_jobs)"

    final_output_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
    echo "Build for $arch completed successfully!"
    echo "Output directory: $final_output_dir"
done

if [ ${#IOS_ARCHES[@]} -gt 1 ]; then
    if ! command -v lipo >/dev/null 2>&1; then
        echo "Skipping universal binary creation: lipo not found in PATH."
    else
        echo ""
        echo "Creating iOS universal binaries..."
        dest_dir="$(platform_universal_dir "$IOS_PLATFORM_KEY" "$BUILD_TYPE")"
        mkdir -p "$dest_dir"

        create_universal_lib() {
            local lib_name=$1
            local suffix=$2
            local -a inputs=()

            for arch in "${IOS_ARCHES[@]}"; do
                local slice_dir
                slice_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
                local candidate="$slice_dir/$lib_name.$suffix"
                if [ -f "$candidate" ]; then
                    inputs+=("$candidate")
                else
                    echo "  Missing slice for $lib_name.$suffix ($arch): $candidate"
                fi
            done

            if [ ${#inputs[@]} -lt 2 ]; then
                echo "  Skipping universal $lib_name.$suffix (need at least two slices)."
                return
            fi

            local dest="$dest_dir/$lib_name.$suffix"
            echo "  lipo -> $dest"
            lipo -create "${inputs[@]}" -output "$dest"
        }

        if [ "$LIB_TYPE" = "Shared" ]; then
            create_universal_lib "libCYLogger" "dylib"
            create_universal_lib "libCYCoroutine" "dylib"
        else
            create_universal_lib "libCYLogger" "a"
            create_universal_lib "libCYCoroutine" "a"
            create_universal_lib "libfmt" "a"
        fi
    fi
fi

cd "$BUILD_DIR"