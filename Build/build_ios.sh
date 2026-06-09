#!/bin/bash

# iOS build helper script for CYLogger
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$BUILD_DIR/.." && pwd)"
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"

BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
REQUESTED_ARCHES_RAW=${3:-"arm64;x86_64"}
REQUESTED_ARCHES_RAW=${REQUESTED_ARCHES_RAW//,/;}

detect_jobs() {
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 4
}

# Pre-copy CYCommon libs for universal fat binary creation
_UNIV_CYCOMMON_NAME=""
_UNIV_DEVICE_SRC=""
_UNIV_DEVICE_DIR=""
_UNIV_SIM_SRC=""
_UNIV_SIM_DIR=""

resolve_ios_arch() {
    local arch=$1
    case "$arch" in
        arm64|device)
            IOS_PLATFORM_TOKEN="OS64"
            IOS_CMAKE_ARCH="arm64"
            ;;
        x86_64|simulator)
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

# Determine the actual filename for a library in an arch directory.
# fmt uses lowercase 'd' for debug suffix (FMT_DEBUG_POSTFIX=d).
resolve_actual_filename() {
    local lib_name=$1
    local build_type=$2
    local suffix=$3

    # fmt uses lowercase d for debug
    if [ "$lib_name" = "fmt" ] && [ "$build_type" = "Debug" ]; then
        echo "lib${lib_name}d.${suffix}"
        return
    fi

    # CYLogger shared dylibs have no "lib" prefix (renamed by build script)
    # CYLoggerStatic and other static libs use lib<name>.a / lib<name>D.a
    case "$lib_name" in
        CYLogger)
            if [ "$suffix" = "dylib" ]; then
                if [ "$build_type" = "Debug" ]; then
                    echo "CYLoggerD.dylib"
                else
                    echo "CYLogger.dylib"
                fi
            fi
            ;;
        CYLoggerStatic)
            # Debug: Src/CMakeLists.txt overrides to "CYLoggerD" -> libCYLoggerD.a
            # Release: root CMakeLists.txt sets "CYLoggerStatic" -> libCYLoggerStatic.a
            # but expected output is libCYLogger.a (no "Static" suffix)
            if [ "$build_type" = "Debug" ]; then
                echo "libCYLoggerD.a"
            else
                echo "libCYLogger.a"
            fi
            ;;
        CYCommon|CYCoroutine)
            if [ "$build_type" = "Debug" ]; then
                echo "lib${lib_name}D.${suffix}"
            else
                echo "lib${lib_name}.${suffix}"
            fi
            ;;
        *)
            if [ "$build_type" = "Debug" ]; then
                echo "lib${lib_name}D.${suffix}"
            else
                echo "lib${lib_name}.${suffix}"
            fi
            ;;
    esac
}

# Normalize dylib: strip CMake version suffixes, then create final symlinks.
# CMake builds: libCYLoggerD.1.0.0.dylib -> libCYLoggerD.1.dylib -> libCYLoggerD.dylib
# Expected output: libCYLoggerD.dylib (real, stripped version) + CYLoggerD.dylib -> libCYLoggerD.dylib
# Same pattern for CYCoroutine: libCYCoroutineD.1.0.0.dylib -> libCYCoroutineD.dylib
#
# CRITICAL: Only remove the unversioned symlink (libX.dylib -> libX.1.dylib).
# Do NOT remove versioned files (libX.1.0.0.dylib, libX.1.dylib) — the real one is our target.
normalize_and_rename_dylibs() {
    local slice_dir=$1
    local build_type=$2

    if [ "$build_type" = "Debug" ]; then
        # === CYLogger (Debug) ===
        # CMake produces: libCYLoggerD.1.0.0.dylib (real) + libCYLoggerD.1.dylib (symlink)
        # Expected: libCYLoggerD.dylib (real, stripped version) + CYLoggerD.dylib -> libCYLoggerD.dylib
        # Remove unversioned symlink first (so mv doesn't fail)
        rm -f "$slice_dir/CYLoggerD.dylib" 2>/dev/null || true
        rm -f "$slice_dir/libCYLoggerD.dylib" 2>/dev/null || true
        # Rename the real versioned file to the unversioned name
        if [ -e "$slice_dir/libCYLoggerD.1.0.0.dylib" ]; then
            mv "$slice_dir/libCYLoggerD.1.0.0.dylib" "$slice_dir/libCYLoggerD.dylib"
        fi
        # Create the expected top-level symlink (only if real file exists)
        if [ -f "$slice_dir/libCYLoggerD.dylib" ]; then
            ln -sf libCYLoggerD.dylib "$slice_dir/CYLoggerD.dylib"
        fi

        # === CYCoroutine (Debug) ===
        rm -f "$slice_dir/libCYCoroutineD.dylib" 2>/dev/null || true
        if [ -e "$slice_dir/libCYCoroutineD.1.0.0.dylib" ]; then
            mv "$slice_dir/libCYCoroutineD.1.0.0.dylib" "$slice_dir/libCYCoroutineD.dylib"
        fi
    else
        # === CYLogger (Release) ===
        rm -f "$slice_dir/CYLogger.dylib" 2>/dev/null || true
        rm -f "$slice_dir/libCYLogger.dylib" 2>/dev/null || true
        if [ -e "$slice_dir/libCYLogger.1.0.0.dylib" ]; then
            mv "$slice_dir/libCYLogger.1.0.0.dylib" "$slice_dir/libCYLogger.dylib"
        fi
        if [ -f "$slice_dir/libCYLogger.dylib" ]; then
            ln -sf libCYLogger.dylib "$slice_dir/CYLogger.dylib"
        fi

        # === CYCoroutine (Release) ===
        rm -f "$slice_dir/libCYCoroutine.dylib" 2>/dev/null || true
        if [ -e "$slice_dir/libCYCoroutine.1.0.0.dylib" ]; then
            mv "$slice_dir/libCYCoroutine.1.0.0.dylib" "$slice_dir/libCYCoroutine.dylib"
        fi
    fi
}

# Copy prerequisite libraries (CYCommon, CYCoroutine, fmt) from their own Bin dirs
# into the CYLogger arch output directory.
copy_prereq_libs() {
    local arch=$1
    local build_type=$2
    local dest_dir=$3
    local lib_type=${4:-"$LIB_TYPE"}

    # Determine the CYCommon/CYCoroutine source arch directory.
    # CYCommon/CYCoroutine build scripts use: arm64, x86_64, arm64-simulator
    # device -> arm64 (device arch), simulator -> x86_64 (simulator arch)
    # arm64-simulator: CYCommon outputs to arm64-simulator/ directory (CYCOMMON_OUTPUT_DIR_IS_FINAL=ON)
    local _src_arch="$arch"
    if [ "$arch" = "device" ]; then
        _src_arch="arm64"
    elif [ "$arch" = "simulator" ]; then
        _src_arch="x86_64"
    fi

    # Copy CYCommon static lib
    # CYCommon outputs to: $_src_arch/$build_type/libCYCommon[|D].a
    local cycommon_name="CYCommon"
    [ "$build_type" = "Debug" ] && cycommon_name="CYCommonD"
    local cycommon_src="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/iOS/$_src_arch/$build_type/lib${cycommon_name}.a"
    if [ -f "$cycommon_src" ]; then
        cp "$cycommon_src" "$dest_dir/"
    fi

    # Copy CYCoroutine static and dylib
    # CYCoroutine outputs to: $_src_arch/$build_type/ (build via add_subdirectory).
    local cycoroutine_static_name="CYCoroutine"
    local cycoroutine_dylib_name="CYCoroutine"
    [ "$build_type" = "Debug" ] && cycoroutine_static_name="CYCoroutineD"
    [ "$build_type" = "Debug" ] && cycoroutine_dylib_name="CYCoroutineD"
    local cycoroutine_src_static="$PROJECT_ROOT/ThirdParty/CYCoroutine/Bin/iOS/$_src_arch/$build_type/lib${cycoroutine_static_name}.a"
    local cycoroutine_src_dylib="$PROJECT_ROOT/ThirdParty/CYCoroutine/Bin/iOS/$_src_arch/$build_type/lib${cycoroutine_dylib_name}.dylib"
    if [ -f "$cycoroutine_src_static" ]; then
        cp "$cycoroutine_src_static" "$dest_dir/"
    fi
    if [ -f "$cycoroutine_src_dylib" ]; then
        cp -L "$cycoroutine_src_dylib" "$dest_dir/"
    fi

    # fmt static lib is already in CYLogger's own output directory from the CMake build
    local fmt_name="fmt"
    [ "$build_type" = "Debug" ] && fmt_name="fmtD"
    local fmt_src="$dest_dir/lib${fmt_name}.a"

    # For arm64-simulator builds: CYCommon outputs to arm64-simulator/$BT/ from standalone build.
    # CYLogger CMake outputs to arm64/$BT/ (CMAKE_OSX_ARCHITECTURES=arm64).
    # We copy the CYLogger dylib from arm64/$BT/ to arm64-simulator/$BT/.
    # restore_arm64_device_dylibs will restore the device dylib after arm64-simulator build.
    if [ "$arch" = "arm64-simulator" ] && [ "$lib_type" = "Shared" ]; then
        local _dest="$PROJECT_ROOT/Bin/iOS/arm64/$build_type"
        mkdir -p "$_dest"
        if [ "$build_type" = "Debug" ]; then
            [ -f "$_dest/CYLoggerD.dylib" ] && cp -L "$_dest/CYLoggerD.dylib" "$dest_dir/" || true
        else
            [ -f "$_dest/CYLogger.dylib" ] && cp -L "$_dest/CYLogger.dylib" "$dest_dir/" || true
        fi
    fi
}

# Restore arm64 device dylibs after arm64-simulator build completes.
# After arm64-simulator cmake build, arm64/ contains the SIMULATOR dylib.
# We need to copy the DEVICE dylib back.
# The device dylib is in device/Debug/ (from the device build that ran before arm64-simulator).
restore_arm64_device_dylibs() {
    local build_type=$1
    local _dest="$PROJECT_ROOT/Bin/iOS/arm64/$build_type"
    local _device_dir="$PROJECT_ROOT/Bin/iOS/device/$build_type"

    if [ "$build_type" = "Debug" ]; then
        [ -f "$_device_dir/CYLoggerD.dylib" ] && cp -L "$_device_dir/CYLoggerD.dylib" "$_dest/CYLoggerD.dylib" || true
    else
        [ -f "$_device_dir/CYLogger.dylib" ] && cp -L "$_device_dir/CYLogger.dylib" "$_dest/CYLogger.dylib" || true
    fi
}

# =============================================================================
# Main build logic
# =============================================================================

declare -a IOS_ARCHES=()
IFS=';' read -r -a _ios_arch_tokens <<< "$REQUESTED_ARCHES_RAW"
for token in "${_ios_arch_tokens[@]}"; do
    token=$(printf '%s' "$token" | tr -d '[:space:]')
    [ -n "$token" ] && IOS_ARCHES+=("$token")
done
unset _ios_arch_tokens
[ ${#IOS_ARCHES[@]} -eq 0 ] && IOS_ARCHES+=("arm64")

IOS_PLATFORM_KEY="ios"
IOS_TOOLCHAIN="$PROJECT_ROOT/cmake/ios.toolchain.cmake"
if [ ! -f "$IOS_TOOLCHAIN" ]; then
    echo "Cannot find iOS toolchain file at $IOS_TOOLCHAIN"
    exit 1
fi
echo "Using iOS toolchain: $IOS_TOOLCHAIN"

if [ "$LIB_TYPE" = "Static" ]; then
    BUILD_KIND_TAG="static"
else
    BUILD_KIND_TAG="shared"
fi

echo "Building CYLogger for iOS..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Architectures: ${IOS_ARCHES[*]}"

# For each arch, build and copy artifacts
for arch in "${IOS_ARCHES[@]}"; do
    resolve_ios_arch "$arch"
    ios_platform="$IOS_PLATFORM_TOKEN"
    cmake_arch="$IOS_CMAKE_ARCH"

    echo ""
    echo "Configuring CMake for $arch..."
    BUILD_SUBDIR="build_ios_${arch}_${BUILD_KIND_TAG}_${BUILD_TYPE}"
    BUILD_PATH="$BUILD_DIR/$BUILD_SUBDIR"
    mkdir -p "$BUILD_PATH"

    # Ensure output directory exists
    _out_dir="$PROJECT_ROOT/Bin/iOS/$arch/$BUILD_TYPE"
    mkdir -p "$_out_dir"

    # Clean stale dylibs in the output directory BEFORE the shared library build.
    # This ensures CMake always generates fresh dylibs (avoiding stale symlink issues).
    # Only clean dylibs, NOT static .a files.
    if [ "$LIB_TYPE" = "Shared" ]; then
        if [ "$BUILD_TYPE" = "Debug" ]; then
            rm -f "$_out_dir/CYLoggerD.dylib" "$_out_dir/libCYLoggerD.dylib" "$_out_dir/libCYLoggerD.1.dylib" "$_out_dir/libCYLoggerD.1.0.0.dylib" 2>/dev/null || true
        else
            rm -f "$_out_dir/CYLogger.dylib" "$_out_dir/libCYLogger.dylib" "$_out_dir/libCYLogger.1.dylib" "$_out_dir/libCYLogger.1.0.0.dylib" 2>/dev/null || true
        fi
    fi

    cmake -S "$PROJECT_ROOT" -B "$BUILD_PATH" \
          -DCMAKE_TOOLCHAIN_FILE="$IOS_TOOLCHAIN" \
          -DPLATFORM="$ios_platform" \
          -DARCHS="$cmake_arch" \
          -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
          -DBUILD_SHARED_LIBS=OFF \
          -DCYLOGGER_BUILD_IOS_DYLIB=ON \
          -DBUILD_STATIC_LIBS=ON \
          -DBUILD_EXAMPLES=OFF \
          -DFMT_DEBUG_POSTFIX=d \
          -DIOS_SIMULATOR_BUILD=$([ "$ios_platform" = "SIMULATORARM64" ] && echo "ON" || echo "OFF") \
          -DTARGET_ARCH="$arch" \
          -DCYCOMMON_OUTPUT_DIR="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/iOS/$arch" \
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$PROJECT_ROOT/Bin/iOS/$arch/$BUILD_TYPE" \
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$PROJECT_ROOT/Bin/iOS/$arch/$BUILD_TYPE" \
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$PROJECT_ROOT/Bin/iOS/$arch/$BUILD_TYPE"

    # Build CYLogger static (always needed)
    echo "Building CYLoggerStatic for $arch..."
    cmake --build "$BUILD_PATH" --target CYLoggerStatic --parallel "$(detect_jobs)"

    # Build CYLogger shared if LIB_TYPE=Shared
    # Build both targets in the same cmake --build call so CMake sees the dependency
    # and processes both correctly.
    if [ "$LIB_TYPE" = "Shared" ]; then
        echo "Building CYLoggerStatic + CYLogger_shared for $arch..."
        cmake --build "$BUILD_PATH" --target CYLoggerStatic CYLogger_shared --parallel "$(detect_jobs)"
    fi

    # Determine output directory
    slice_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"

    # Normalize and rename dylibs (strip version suffixes, clean up CMake version symlinks)
    normalize_and_rename_dylibs "$slice_dir" "$BUILD_TYPE"
    # Clean up CMake version symlinks that remain after renaming (libX.1.dylib is a symlink to libX.1.0.0.dylib)
    rm -f "$slice_dir"/{libCYLoggerD,libCYLogger,libCYCoroutineD,libCYCoroutine}.1.dylib 2>/dev/null || true

    # Rename libCYLoggerStatic.a -> libCYLogger.a for Release (expected output name)
    if [ "$BUILD_TYPE" = "Release" ]; then
        [ -f "$slice_dir/libCYLoggerStatic.a" ] && mv "$slice_dir/libCYLoggerStatic.a" "$slice_dir/libCYLogger.a" || true
    fi

    # For device/simulator builds (single-arch), copy CYLogger dylib to output dir.
    # CYLogger CMake outputs to arm64/ (device) or x86_64/ (simulator).
    # The output directory (device/ or simulator/) is separate.
    if [ "$LIB_TYPE" = "Shared" ]; then
        _src_dir="$slice_dir"
        if [ "$arch" = "device" ]; then
            _src_dir="$PROJECT_ROOT/Bin/iOS/arm64/$BUILD_TYPE"
        elif [ "$arch" = "simulator" ]; then
            _src_dir="$PROJECT_ROOT/Bin/iOS/x86_64/$BUILD_TYPE"
        fi
        if [ "$BUILD_TYPE" = "Debug" ]; then
            [ -f "$_src_dir/CYLoggerD.dylib" ] && cp -L "$_src_dir/CYLoggerD.dylib" "$slice_dir/" || true
        else
            [ -f "$_src_dir/CYLogger.dylib" ] && cp -L "$_src_dir/CYLogger.dylib" "$slice_dir/" || true
        fi
    fi

    # Copy prerequisite libs into CYLogger's output directory
    # For Static builds, use lib_type=Shared so CYCoroutine dylibs get copied too
    if [ "$LIB_TYPE" = "Static" ]; then
        copy_prereq_libs "$arch" "$BUILD_TYPE" "$slice_dir" "Shared"
    else
        copy_prereq_libs "$arch" "$BUILD_TYPE" "$slice_dir" "$LIB_TYPE"
    fi

    # Restore arm64 device dylibs after arm64-simulator build.
    # The arm64-simulator CMake build overwrites arm64/ directory,
    # so we must copy the real dylib files back.
    if [ "$arch" = "arm64-simulator" ] && [ "$LIB_TYPE" = "Shared" ]; then
        restore_arm64_device_dylibs "$BUILD_TYPE"
    fi

    echo "Build for $arch completed successfully!"
    echo "Output directory: $slice_dir"
done

# =============================================================================
# Create universal (fat) binaries from individual arch slices.
# Uses two-step lipo to handle arm64-device and arm64-simulator conflict:
# Step 1: combine arm64-device + arm64-simulator into a temp arm64-fat binary
# Step 2: combine that temp with x86_64 into the final universal binary
# =============================================================================
if [ ${#IOS_ARCHES[@]} -gt 1 ]; then
    if ! command -v lipo >/dev/null 2>&1; then
        echo "Skipping universal binary creation: lipo not found."
    else
        echo ""
        echo "Creating iOS universal binaries..."
        dest_dir="$(platform_universal_dir "$IOS_PLATFORM_KEY" "$BUILD_TYPE")"
        mkdir -p "$dest_dir"

        # Pre-copy CYCommon libs to arch directories for fat binary creation.
        # CYCommon outputs to arm64/$BT/ and x86_64/$BT/ (standalone build).
        _UNIV_CYCOMMON_NAME="libCYCommon.a"
        [ "$BUILD_TYPE" = "Debug" ] && _UNIV_CYCOMMON_NAME="libCYCommonD.a"
        _UNIV_DEVICE_SRC="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/iOS/arm64/$BUILD_TYPE/$_UNIV_CYCOMMON_NAME"
        _UNIV_DEVICE_DIR="$PROJECT_ROOT/Bin/iOS/device/$BUILD_TYPE"
        _UNIV_SIM_SRC="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/iOS/x86_64/$BUILD_TYPE/$_UNIV_CYCOMMON_NAME"
        _UNIV_SIM_DIR="$PROJECT_ROOT/Bin/iOS/simulator/$BUILD_TYPE"
        [ -f "$_UNIV_DEVICE_SRC" ] && mkdir -p "$_UNIV_DEVICE_DIR" && cp "$_UNIV_DEVICE_SRC" "$_UNIV_DEVICE_DIR/"
        [ -f "$_UNIV_SIM_SRC" ] && mkdir -p "$_UNIV_SIM_DIR" && cp "$_UNIV_SIM_SRC" "$_UNIV_SIM_DIR/"

        # Create a universal binary from slices that already have the correct name.
        # Also normalizes dylibs: strips CMake version suffixes (libX.1.0.0.dylib -> libX.dylib).
        # Used for CYLogger dylibs where the expected output name differs from CMake's naming.
        create_universal_lib_from_name() {
            local expected_name=$1
            local -a inputs=()
            for arch in "${IOS_ARCHES[@]}"; do
                if [ "$arch" = "arm64-simulator" ]; then
                    continue
                fi
                local arch_slice_dir
                arch_slice_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"

                # Look for the stripped dylib first (libX.dylib), then fall back to versioned.
                # Prefer the normalized file created by normalize_and_rename_dylibs.
                local candidate="$arch_slice_dir/$expected_name"
                if [ ! -f "$candidate" ]; then
                    # Try the versioned .1.0.0.dylib specifically (most specific version)
                    local stripped_base="${expected_name%.dylib}"
                    local ver_file="$arch_slice_dir/${stripped_base}.1.0.0.dylib"
                    if [ -f "$ver_file" ]; then
                        local real_file="$arch_slice_dir/$stripped_base.dylib"
                        cp "$ver_file" "$real_file"
                        candidate="$real_file"
                    else
                        # Fallback: find any versioned variant
                        candidate=$(ls "$arch_slice_dir/${stripped_base}".*.dylib 2>/dev/null | head -1)
                        if [ -n "$candidate" ] && [ -f "$candidate" ]; then
                            local real_file="$arch_slice_dir/$stripped_base.dylib"
                            cp "$candidate" "$real_file"
                            candidate="$real_file"
                        else
                            candidate=""
                        fi
                    fi
                fi
                if [ -n "$candidate" ] && [ -f "$candidate" ]; then
                    inputs+=("$candidate")
                fi
            done

            if [ ${#inputs[@]} -lt 2 ]; then
                echo "  Skipping $expected_name: not enough slices (have ${#inputs[@]})"
                return
            fi

            local output="$dest_dir/$expected_name"
            if lipo -create "${inputs[@]}" -output "$output" 2>/dev/null; then
                echo "  Created universal: $output (${#inputs[@]} slices)"
                # Clean up versioned files from source directories after creating universal
                for inp in "${inputs[@]}"; do
                    rm -f "${inp%.dylib}.1.dylib" "${inp%.dylib}.1.0.0.dylib" 2>/dev/null || true
                done
                # Clean up versioned files in the universal output directory
                rm -f "$dest_dir"/{libCYLoggerD,libCYLogger}.1.dylib "$dest_dir"/{libCYLoggerD,libCYLogger}.1.0.0.dylib 2>/dev/null || true
                # Create expected symlinks for CYLogger dylibs
                if [[ "$expected_name" == libCYLogger* ]]; then
                    local stripped_base="${expected_name%.dylib}"
                    if [ "$BUILD_TYPE" = "Debug" ]; then
                        rm -f "$dest_dir/CYLoggerD.dylib" 2>/dev/null || true
                        ln -sf "$(basename "$expected_name")" "$dest_dir/CYLoggerD.dylib"
                        echo "  Created symlink: $dest_dir/CYLoggerD.dylib"
                    else
                        rm -f "$dest_dir/CYLogger.dylib" 2>/dev/null || true
                        ln -sf "$(basename "$expected_name")" "$dest_dir/CYLogger.dylib"
                        echo "  Created symlink: $dest_dir/CYLogger.dylib"
                    fi
                fi
            else
                echo "  Failed: lipo for $expected_name"
            fi
        }

        create_universal_lib() {
            local lib_name=$1
            local suffix=$2
            local actual_name
            actual_name=$(resolve_actual_filename "$lib_name" "$BUILD_TYPE" "$suffix")

            # Collect slices for universal binary.
            # arm64-device and arm64-simulator BOTH produce arm64 architecture slices.
            # lipo -create CANNOT combine two arm64 slices (same arch).
            # Solution: use ONLY arm64-device + x86_64 for universal (skip arm64-simulator).
            # arm64-simulator builds go to their own arm64-simulator/ directory.
            local -a inputs=()
            for arch in "${IOS_ARCHES[@]}"; do
                if [ "$arch" = "arm64-simulator" ]; then
                    continue
                fi
                local arch_slice_dir
                arch_slice_dir="$(platform_slice_dir "$IOS_PLATFORM_KEY" "$arch" "$BUILD_TYPE")"
                local candidate="$arch_slice_dir/$actual_name"
                if [ -f "$candidate" ]; then
                    inputs+=("$candidate")
                fi
            done

            if [ ${#inputs[@]} -lt 2 ]; then
                echo "  Skipping $actual_name: not enough slices (need 2, have ${#inputs[@]})"
                return
            fi

            local output="$dest_dir/$actual_name"
            if lipo -create "${inputs[@]}" -output "$output" 2>/dev/null; then
                echo "  Created universal: $output (${#inputs[@]} slices: $(echo "${inputs[@]}" | tr ' ' '\n' | xargs -I{} basename {} | tr '\n' ' '))"
            else
                echo "  Failed: lipo for $actual_name"
            fi
        }

        # Static libs
        create_universal_lib "CYCommon" "a"
        create_universal_lib "CYCoroutine" "a"
        create_universal_lib "CYLoggerStatic" "a"
        create_universal_lib "fmt" "a"

        # Shared dylibs
        if [ "$LIB_TYPE" = "Shared" ]; then
            # CYLogger dylibs: expected name is libCYLoggerD.dylib / libCYLogger.dylib
            if [ "$BUILD_TYPE" = "Debug" ]; then
                create_universal_lib_from_name "libCYLoggerD.dylib"
            else
                create_universal_lib_from_name "libCYLogger.dylib"
            fi
            create_universal_lib "CYCoroutine" "dylib"
        fi
    fi
fi
