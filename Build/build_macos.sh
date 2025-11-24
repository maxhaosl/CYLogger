#!/bin/bash

# macOS build helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
MACOS_PLATFORM_KEY="macos"
MACOS_PLATFORM_DIR="$(map_platform_dir "$MACOS_PLATFORM_KEY")"
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
REQUESTED_ARCH=${CMAKE_OSX_ARCHITECTURES:-"x86_64;arm64"}
declare -a MACOS_ARCHES=()
IFS=';' read -r -a _macos_arch_tokens <<< "$REQUESTED_ARCH"
for token in "${_macos_arch_tokens[@]}"; do
    token=$(printf '%s' "$token" | tr -d '[:space:]')
    if [ -n "$token" ]; then
        MACOS_ARCHES+=("$token")
    fi
done
unset _macos_arch_tokens
if [ ${#MACOS_ARCHES[@]} -eq 0 ]; then
    MACOS_ARCHES+=("$(uname -m)")
fi

detect_jobs() {
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu 2>/dev/null && return
    fi
    echo 4
}

echo "Building CYLogger for macOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Architectures: ${REQUESTED_ARCH//;/, }"

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_SHARED="OFF"
    BUILD_KIND_TAG="static"
else
    BUILD_SHARED="ON"
    BUILD_KIND_TAG="shared"
fi

# Ensure CYCoroutine dependencies are built first
ensure_cycoroutine_macos() {
    local arch_name=$1
    if [ -z "$arch_name" ]; then
        arch_name=$(uname -m)
    fi

    local arch_dir
    arch_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch_name" "$BUILD_TYPE")"
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

    local shared_flag
    if [ "$LIB_TYPE" = "Shared" ]; then
        shared_flag="ON"
    else
        shared_flag="OFF"
    fi

    # Build dependency for single architecture (first one if multiple specified)
    local dep_dir="$BUILD_DIR/deps_macos_${arch_name}_${BUILD_TYPE}_${shared_flag}"
    local -a dep_cmake_args=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=$shared_flag"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
        "-DTARGET_ARCH=$arch_name"
        "-DCMAKE_OSX_ARCHITECTURES=$arch_name"
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

for arch in "${MACOS_ARCHES[@]}"; do
    echo ""
    echo "Configuring CMake for $arch..."
    BUILD_SUBDIR="build_macos_${arch}_${BUILD_KIND_TAG}_${BUILD_TYPE}"
    BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
    mkdir -p "$BUILD_PATH"

    cmake -S "$PROJECT_ROOT" -B "$BUILD_PATH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS="$BUILD_SHARED" \
          -DBUILD_EXAMPLES=ON \
          -DTARGET_ARCH="$arch" \
          -DCMAKE_OSX_ARCHITECTURES="$arch"

    ensure_cycoroutine_macos "$arch"

    echo "Building for $arch..."
    cmake --build "$BUILD_PATH" --parallel "$(detect_jobs)"

    final_output_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
    echo "Build for $arch completed successfully!"
    echo "Output directory: $final_output_dir"
done

# Produce universal binaries if more than one architecture slice was built
if [ ${#MACOS_ARCHES[@]} -gt 1 ]; then
    if ! command -v lipo >/dev/null 2>&1; then
        echo "Skipping universal binary creation: lipo not found in PATH."
    else
        echo ""
        echo "Creating universal binaries..."
        dest_dir="$(platform_universal_dir "$MACOS_PLATFORM_KEY" "$BUILD_TYPE")"
        mkdir -p "$dest_dir"

        create_universal_lib() {
            local lib_name=$1
            local suffix=$2
            local -a inputs=()
            local arch_missing=0

            for arch in "${MACOS_ARCHES[@]}"; do
                local slice_dir
                slice_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
                local candidate="$slice_dir/$lib_name.$suffix"
                if [ -f "$candidate" ]; then
                    inputs+=("$candidate")
                else
                    echo "  Missing slice for $lib_name.$suffix ($arch): $candidate"
                    arch_missing=1
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
