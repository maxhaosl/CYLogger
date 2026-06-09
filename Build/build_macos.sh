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
# Canonical output directory name is "MacOS" (not "macOS")
CYCOMMON_PLATFORM_DIR="MacOS"
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

# Ensure CYCommon submodule exists
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

ensure_cycommon_submodule || exit 1

detect_jobs() {
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu 2>/dev/null && return
    fi
    echo 4
}

# Compute the expected CYCommon library path for a given architecture and build type.
# This must match what ensure_cycommon_macos produces.
cycommon_lib_path() {
    local arch_name=$1
    local lib_suffix=""
    [ "$BUILD_TYPE" = "Debug" ] && lib_suffix="D"
    echo "$PROJECT_ROOT/ThirdParty/CYCommon/Bin/$CYCOMMON_PLATFORM_DIR/$arch_name/$BUILD_TYPE/libCYCommon${lib_suffix}.a"
}

# Build CYCommon static library for macOS using the dedicated CYCommon build script.
# CYLogger CMakeLists.txt (Unix section) searches for CYCommon at
# Bin/macOS/<arch>/<BT>/libCYCommon.a, so we set CYCOMMON_OUTPUT_DIR
# to the parent of that path so CYCommon writes libCYCommon.a directly into it.
ensure_cycommon_macos() {
    local arch_name=$1
    local arch_dir
    arch_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch_name" "$BUILD_TYPE")"
    local lib_suffix=""
    if [ "$BUILD_TYPE" = "Debug" ]; then
        lib_suffix="D"
    fi
    local sta_check="$arch_dir/libCYCommon${lib_suffix}.a"

    if [ -f "$sta_check" ]; then
        echo "CYCommon already present: $sta_check"
        return
    fi

    # CYCOMMON_OUTPUT_DIR is the complete final path (e.g. CYCommon/Bin/MacOS/x86_64/Debug) with IS_FINAL=ON.
    # CYCommon will write libCYCommon[|D].a directly into this directory (no extra BT subdirectory).
    local cycommon_out_parent="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/$CYCOMMON_PLATFORM_DIR/$arch_name/$BUILD_TYPE"
    mkdir -p "$cycommon_out_parent"

    echo "Building CYCommon for macOS $arch_name/$BUILD_TYPE (output: $cycommon_out_parent)..."
    CYCOMMON_OUTPUT_DIR="$cycommon_out_parent" CYCOMMON_OUTPUT_DIR_IS_FINAL=ON \
        bash "$PROJECT_ROOT/ThirdParty/CYCommon/Build/build_all_mac.sh" \
        "$BUILD_TYPE" "OFF" "$arch_name"
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
# CYLogger links CYCoroutine_static; if LIB_TYPE=Shared, also build CYCoroutine_shared for the output.
ensure_cycoroutine_macos() {
    local arch_name=$1
    if [ -z "$arch_name" ]; then
        arch_name=$(uname -m)
    fi

    local arch_dir
    arch_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch_name" "$BUILD_TYPE")"
    local lib_suffix=""
    if [ "$BUILD_TYPE" = "Debug" ]; then
        lib_suffix="D"
    fi
    local static_lib="$arch_dir/libCYCoroutine${lib_suffix}.a"
    local shared_lib="$arch_dir/libCYCoroutine${lib_suffix}.dylib"

    local need_static=0 need_shared=0
    [ ! -f "$static_lib" ] && need_static=1
    # Build CYCoroutine_shared only when CYLogger itself is built as shared (for output completeness)
    if [ "$LIB_TYPE" = "Shared" ] && [ ! -f "$shared_lib" ]; then
        need_shared=1
    fi

    if [ "$need_static" -eq 0 ] && [ "$need_shared" -eq 0 ]; then
        echo "CYCoroutine dependencies already present for ${arch_name}/${BUILD_TYPE}/${LIB_TYPE}"
        return
    fi

    echo "Building CYCoroutine dependencies for ${arch_name}/${BUILD_TYPE}/${LIB_TYPE}..."

    local dep_dir="$BUILD_DIR/deps_macos_${arch_name}_${BUILD_TYPE}_${LIB_TYPE}"
    local _cycommon_lib
    _cycommon_lib="$(cycommon_lib_path "$arch_name")"

    # Ensure the output directory exists so fmt/ar don't fail
    local _dep_output_dir
    _dep_output_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch_name" "$BUILD_TYPE")"
    mkdir -p "$_dep_output_dir"

    # CYLogger_shared links CYCoroutine_static, but we also produce CYCoroutine_shared for the output
    local _dep_build_shared="OFF"
    [ "$LIB_TYPE" = "Shared" ] && _dep_build_shared="ON"

    local -a dep_cmake_args=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DBUILD_SHARED_LIBS=$_dep_build_shared"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
        "-DTARGET_ARCH=$arch_name"
        "-DCMAKE_OSX_ARCHITECTURES=$arch_name"
        "-DCYCOMMON_LIB_PATH=$_cycommon_lib"
    )

    rm -rf "$dep_dir"
    mkdir -p "$dep_dir"
    cmake -S "$PROJECT_ROOT" -B "$dep_dir" "${dep_cmake_args[@]}"
    if [ "$need_static" -eq 1 ]; then
        cmake --build "$dep_dir" --target CYCoroutine_static --parallel "$(detect_jobs)"
    fi
    if [ "$need_shared" -eq 1 ]; then
        cmake --build "$dep_dir" --target CYCoroutine_shared --parallel "$(detect_jobs)"
    fi
}

for arch in "${MACOS_ARCHES[@]}"; do
    echo ""
    echo "Configuring CMake for $arch..."

    # Compute lib_suffix at loop level (matches ensure_cycommon_macos logic)
    loop_lib_suffix=""
    [ "$BUILD_TYPE" = "Debug" ] && loop_lib_suffix="D"

    # Build CYCommon FIRST so cmake configure can find the correct-architecture library
    ensure_cycommon_macos "$arch"
    # Copy CYCommon static library to CYLogger's output directory for this arch/build type
    final_output_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
    mkdir -p "$final_output_dir"
    _cc_lib_src="$(cycommon_lib_path "$arch")"
    _cc_lib_dest="$final_output_dir/libCYCommon${loop_lib_suffix}.a"
    if [ -f "$_cc_lib_src" ] && [ ! -f "$_cc_lib_dest" ]; then
        cp "$_cc_lib_src" "$_cc_lib_dest"
        echo "Copied CYCommon -> $_cc_lib_dest"
    fi
    ensure_cycoroutine_macos "$arch"

    # CYLOGGER_WANT_SHARED is used by CYLogger's Src/CMakeLists.txt directly
    # (BUILD_SHARED_LIBS may be overridden by fmt/CYCommon blocks)
    _shared_flag="OFF" _cylogger_want_shared="OFF"
    [ "$LIB_TYPE" = "Shared" ] && _shared_flag="ON" && _cylogger_want_shared="ON"
    BUILD_SUBDIR="build_macos_${arch}_${BUILD_KIND_TAG}_${BUILD_TYPE}"
    BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
    mkdir -p "$BUILD_PATH"

    _cycommon_lib="$(cycommon_lib_path "$arch")"
    cmake -S "$PROJECT_ROOT" -B "$BUILD_PATH" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          "-DBUILD_SHARED_LIBS=$_shared_flag" \
          "-DCYLOGGER_WANT_SHARED=$_cylogger_want_shared" \
          -DBUILD_EXAMPLES=ON \
          -DTARGET_ARCH="$arch" \
          -DCMAKE_OSX_ARCHITECTURES="$arch" \
          -DCYCOMMON_LIB_PATH="$_cycommon_lib"

    echo "Building CYLogger for $arch..."
    if [ "$LIB_TYPE" = "Static" ]; then
        # Build only CYLoggerStatic (CYLogger_shared may also be built as a dependency
        # due to CYLoggerStatic linking CYLogger::shared, but we only need the static)
        cmake --build "$BUILD_PATH" --target CYLoggerStatic --parallel "$(detect_jobs)"
    else
        cmake --build "$BUILD_PATH" --target CYLogger_shared --parallel "$(detect_jobs)"
    fi

    final_output_dir="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
    echo "Build for $arch completed successfully!"
    echo "Output directory: $final_output_dir"
done

# Produce universal binaries if more than one architecture slice was built
    if [ ${#MACOS_ARCHES[@]} -gt 1 ]; then
        if command -v lipo >/dev/null 2>&1; then
            echo ""
            echo "Creating universal binaries..."
            dest_dir="$(platform_universal_dir "$MACOS_PLATFORM_KEY" "$BUILD_TYPE")"
            mkdir -p "$dest_dir"
        _lib_suffix=""
        [ "$BUILD_TYPE" = "Debug" ] && _lib_suffix="D"

        if [ "$LIB_TYPE" = "Shared" ]; then
            # CYLogger shared: libCYLoggerD.dylib (Debug), libCYLogger.dylib (Release)
            arm_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$BUILD_TYPE")/libCYLogger${_lib_suffix}.dylib"
            x86_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$BUILD_TYPE")/libCYLogger${_lib_suffix}.dylib"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libCYLogger${_lib_suffix}.dylib"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libCYLogger${_lib_suffix}.dylib"
            fi
            # CYCoroutine shared
            arm_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$BUILD_TYPE")/libCYCoroutine${_lib_suffix}.dylib"
            x86_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$BUILD_TYPE")/libCYCoroutine${_lib_suffix}.dylib"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libCYCoroutine${_lib_suffix}.dylib"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libCYCoroutine${_lib_suffix}.dylib"
            fi
        else
            # CYLogger static
            arm_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$BUILD_TYPE")/libCYLogger${_lib_suffix}.a"
            x86_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$BUILD_TYPE")/libCYLogger${_lib_suffix}.a"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libCYLogger${_lib_suffix}.a"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libCYLogger${_lib_suffix}.a"
            fi
            # CYCommon static - CYCommon is built to its own Bin directory
            arm_slice="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/$CYCOMMON_PLATFORM_DIR/arm64/$BUILD_TYPE/libCYCommon${_lib_suffix}.a"
            x86_slice="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/$CYCOMMON_PLATFORM_DIR/x86_64/$BUILD_TYPE/libCYCommon${_lib_suffix}.a"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libCYCommon${_lib_suffix}.a"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libCYCommon${_lib_suffix}.a"
            fi
            # CYCoroutine static
            arm_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$BUILD_TYPE")/libCYCoroutine${_lib_suffix}.a"
            x86_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$BUILD_TYPE")/libCYCoroutine${_lib_suffix}.a"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libCYCoroutine${_lib_suffix}.a"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libCYCoroutine${_lib_suffix}.a"
            fi
            # fmt static
            _fmt_suffix=""
            [ "$BUILD_TYPE" = "Debug" ] && _fmt_suffix="d"
            arm_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$BUILD_TYPE")/libfmt${_fmt_suffix}.a"
            x86_slice="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$BUILD_TYPE")/libfmt${_fmt_suffix}.a"
            if [ -f "$arm_slice" ] && [ -f "$x86_slice" ]; then
                echo "  lipo -> $dest_dir/libfmt${_fmt_suffix}.a"
                lipo -create "$arm_slice" "$x86_slice" -output "$dest_dir/libfmt${_fmt_suffix}.a"
            fi
        fi
    else
        echo "Skipping universal binary creation: lipo not found in PATH."
    fi
fi

cd "$BUILD_DIR"
