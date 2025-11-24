#!/bin/bash

# macOS build helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
REQUESTED_ARCH=${CMAKE_OSX_ARCHITECTURES:-"x86_64;arm64"}

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
    BUILD_SUBDIR="build_macos_static_$BUILD_TYPE"
    BUILD_SHARED="OFF"
else
    BUILD_SUBDIR="build_macos_shared_$BUILD_TYPE"
    BUILD_SHARED="ON"
fi

BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
mkdir -p "$BUILD_PATH"
cd "$BUILD_PATH"

echo "Configuring CMake..."
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DBUILD_SHARED_LIBS="$BUILD_SHARED" \
      -DBUILD_EXAMPLES=ON \
      ${CMAKE_OSX_ARCHITECTURES:+-DCMAKE_OSX_ARCHITECTURES="$CMAKE_OSX_ARCHITECTURES"} \
      "$PROJECT_ROOT"

# Ensure CYCoroutine dependencies are built first
ensure_cycoroutine_macos() {
    local requested_arch="${CMAKE_OSX_ARCHITECTURES:-$(uname -m)}"
    
    # For multiple architectures, use first architecture for dependency build
    # (CYCoroutine will be built as universal binary by main build)
    local arch_name
    if [[ "$requested_arch" == *";"* ]]; then
        # Extract first architecture from list
        arch_name="${requested_arch%%;*}"
        echo "Note: Multiple architectures requested ($requested_arch), using $arch_name for CYCoroutine dependency"
    else
        arch_name="$requested_arch"
    fi
    
    local arch_dir="$PROJECT_ROOT/Bin/macOS/$arch_name/$BUILD_TYPE"
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

ensure_cycoroutine_macos

echo "Building..."
cmake --build . --parallel "$(detect_jobs)"

# Determine output directory based on architecture
output_arch=""
if [ -n "${CMAKE_OSX_ARCHITECTURES:-}" ]; then
    if [[ "$CMAKE_OSX_ARCHITECTURES" == *";"* ]]; then
        # Multiple architectures - use "universal" or first architecture
        output_arch="universal"
    else
        output_arch="$CMAKE_OSX_ARCHITECTURES"
    fi
else
    output_arch=$(uname -m)
fi

echo "Build completed successfully!"
echo "Output directory: $PROJECT_ROOT/Bin/macOS/$output_arch/$BUILD_TYPE"

TEST_BIN="$PROJECT_ROOT/Bin/macOS/$output_arch/$BUILD_TYPE/CYLoggerConsoleTest"
if [ -f "$TEST_BIN" ]; then
    echo ""
    echo "Running console test..."
    if ! "$TEST_BIN"; then
        echo "Warning: Test execution failed, but build completed successfully."
    fi
fi

cd "$BUILD_DIR"
