#!/bin/bash

# Build all macOS variants (Debug/Release, Static/Shared, x86_64/arm64/universal)
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
MACOS_PLATFORM_KEY="macos"
MACOS_PLATFORM_DIR="$(map_platform_dir "$MACOS_PLATFORM_KEY")"
SCRIPT_NAME="$(basename "$0")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Building all macOS variants for CYLogger"
echo "=========================================="
echo ""

# Track failures
FAILED_BUILDS=()
SUCCESSFUL_BUILDS=()

# Helper: lipo two slices into a universal binary (bash 3.2 compatible, no local function)
do_lipo() {
    local src_arm=$1
    local src_x86=$2
    local dest=$3
    if [ -f "$src_arm" ] && [ -f "$src_x86" ]; then
        echo -e "  lipo $dest"
        lipo -create "$src_arm" "$src_x86" -output "$dest"
    else
        echo -e "${RED}  Warning: missing inputs for $(basename "$dest")${NC}"
        [ ! -f "$src_arm" ] && echo -e "    Missing: $src_arm"
        [ ! -f "$src_x86" ] && echo -e "    Missing: $src_x86"
    fi
}

# Function to run a build and track results
run_build() {
    local build_type=$1
    local lib_type=$2
    local arch=$3
    local description="$build_type $lib_type ($arch)"

    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo -e "${YELLOW}Building: $description${NC}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    if CMAKE_OSX_ARCHITECTURES="$arch" bash "$BUILD_DIR/build_macos.sh" "$build_type" "$lib_type"; then
        echo -e "${GREEN}✓ Success: $description${NC}"
        SUCCESSFUL_BUILDS+=("$description")
        return 0
    else
        echo -e "${RED}✗ Failed: $description${NC}"
        FAILED_BUILDS+=("$description")
        return 1
    fi
}

# Function to combine universal libraries using lipo
combine_universal() {
    local build_type=$1
    local lib_type=$2

    if ! command -v lipo >/dev/null 2>&1; then
        echo -e "${RED}Error: lipo command not found; cannot produce universal binaries.${NC}"
        return 1
    fi

    local suffix
    if [ "$lib_type" = "Shared" ]; then
        suffix="dylib"
    else
        suffix="a"
    fi

    local lib_suffix=""
    if [ "$build_type" = "Debug" ]; then
        lib_suffix="D"
    fi

    # CYLogger static output name: CYLoggerD (Debug on non-Windows), CYLogger (all others)
    local logger_suffix=""
    if [ "$lib_type" = "Static" ] && [ "$build_type" = "Debug" ]; then
        logger_suffix="D"
    fi

    local dest_dir
    dest_dir="$(platform_universal_dir "$MACOS_PLATFORM_KEY" "$build_type")"
    mkdir -p "$dest_dir"

    if [ "$lib_type" = "Shared" ]; then
        # CYLogger_shared outputs libCYLoggerD.dylib (Debug) or libCYLogger.dylib (Release)
        # CYCoroutine_shared outputs libCYCoroutineD.dylib (Debug) or libCYCoroutine.dylib (Release)
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYLogger${lib_suffix}.dylib" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYLogger${lib_suffix}.dylib" \
            "$dest_dir/libCYLogger${lib_suffix}.dylib"
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYCoroutine${lib_suffix}.dylib" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYCoroutine${lib_suffix}.dylib" \
            "$dest_dir/libCYCoroutine${lib_suffix}.dylib"
    else
        # Static libraries
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYLogger${logger_suffix}.${suffix}" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYLogger${logger_suffix}.${suffix}" \
            "$dest_dir/libCYLogger${logger_suffix}.${suffix}"
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYCommon${lib_suffix}.${suffix}" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYCommon${lib_suffix}.${suffix}" \
            "$dest_dir/libCYCommon${lib_suffix}.${suffix}"
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYCoroutine${lib_suffix}.${suffix}" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYCoroutine${lib_suffix}.${suffix}" \
            "$dest_dir/libCYCoroutine${lib_suffix}.${suffix}"
        # fmt Debug uses lowercase 'd' suffix (libfmtd.a); Release uses no suffix (libfmt.a)
        local fmt_suffix=""
        [ "$build_type" = "Debug" ] && fmt_suffix="d"
        do_lipo \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libfmt${fmt_suffix}.${suffix}" \
            "$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libfmt${fmt_suffix}.${suffix}" \
            "$dest_dir/libfmt${fmt_suffix}.${suffix}"
    fi
}

# Build all single-architecture variants first
# 1. Debug Static
run_build "Debug" "Static" "x86_64"
run_build "Debug" "Static" "arm64"

# 2. Debug Shared
run_build "Debug" "Shared" "x86_64"
run_build "Debug" "Shared" "arm64"

# 3. Release Static
run_build "Release" "Static" "x86_64"
run_build "Release" "Static" "arm64"

# 4. Release Shared
run_build "Release" "Shared" "x86_64"
run_build "Release" "Shared" "arm64"

# Now combine universal libraries
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Creating universal libraries...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

combine_universal "Debug" "Static"
combine_universal "Debug" "Shared"
combine_universal "Release" "Static"
combine_universal "Release" "Shared"

# Propagate CYCoroutine dylibs to CYCoroutine's own standalone Bin directory.
# CYLogger builds CYCoroutine_shared via CMake deps, but the artifacts land in
# CYLogger/Bin/MacOS/<arch>/<BT>/.  Standalone CYCoroutine builds expect them
# under CYCoroutine/Bin/MacOS/<arch>/<BT>/, so we copy them over here.
propagate_cycoroutine_dylibs() {
    local src_base="$OUTPUT_BASE/MacOS"
    local dst_base="$PROJECT_ROOT/ThirdParty/CYCoroutine/Bin/MacOS"

    for build_type in Debug Release; do
        local lib_suffix=""
        [ "$build_type" = "Debug" ] && lib_suffix="D"

        for arch in arm64 x86_64 universal; do
            local src_dir="$src_base/$arch/$build_type"
            local dst_dir="$dst_base/$arch/$build_type"
            mkdir -p "$dst_dir"

            # Copy the primary dylib (libCYCoroutineD.dylib / libCYCoroutine.dylib)
            local primary_src="$src_dir/libCYCoroutine${lib_suffix}.dylib"
            local primary_dst="$dst_dir/libCYCoroutine${lib_suffix}.dylib"
            if [ -f "$primary_src" ]; then
                cp "$primary_src" "$primary_dst"
                echo "  Copied CYCoroutine dylib -> $primary_dst"
            fi
        done
    done
}

ensure_primary_dylib_aliases() {
    local base_dir=$1
    local library_base=$2

    for build_type in Debug Release; do
        local lib_suffix=""
        [ "$build_type" = "Debug" ] && lib_suffix="D"

        for arch in arm64 x86_64 universal; do
            local out_dir="$base_dir/$arch/$build_type"
            [ -d "$out_dir" ] || continue

            local primary="$out_dir/lib${library_base}${lib_suffix}.dylib"
            local versioned="$out_dir/lib${library_base}${lib_suffix}.1.0.0.dylib"
            local soversion="$out_dir/lib${library_base}${lib_suffix}.1.dylib"

            if [ -f "$primary" ]; then
                continue
            fi

            if [ -f "$versioned" ]; then
                cp "$versioned" "$primary"
                echo "  Created unversioned dylib copy -> $primary"
            elif [ -L "$primary" ] || [ -f "$soversion" ]; then
                cp -L "$soversion" "$primary"
                echo "  Materialized dylib alias -> $primary"
            fi
        done
    done
}

# Note: CYCoroutine dylibs are propagated AFTER all builds complete (see below).

# Also build universal variants directly (CMake can handle it)
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Building universal variants directly...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

run_build "Debug" "Static" "x86_64;arm64"
run_build "Debug" "Shared" "x86_64;arm64"
run_build "Release" "Static" "x86_64;arm64"
run_build "Release" "Shared" "x86_64;arm64"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Normalizing dylib output names...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
ensure_primary_dylib_aliases "$OUTPUT_BASE/MacOS" "CYCoroutine"
ensure_primary_dylib_aliases "$OUTPUT_BASE/MacOS" "CYLogger"

# Propagate CYCoroutine dylibs to CYCoroutine's own standalone Bin directory.
# CYLogger builds CYCoroutine_shared via CMake deps, but the artifacts land in
# CYLogger/Bin/MacOS/<arch>/<BT>/.  Standalone CYCoroutine builds expect them
# under CYCoroutine/Bin/MacOS/<arch>/<BT>/, so we copy them over here.
# This must run AFTER all builds complete so CYCommon's CYCOMMON_OUTPUT_DIR_IS_FINAL
# cleanup does not overwrite them.
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Propagating CYCoroutine dylibs to standalone output...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
propagate_cycoroutine_dylibs
ensure_primary_dylib_aliases "$PROJECT_ROOT/ThirdParty/CYCoroutine/Bin/MacOS" "CYCoroutine"

# Print summary
echo ""
echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo -e "${GREEN}Successful builds: ${#SUCCESSFUL_BUILDS[@]}${NC}"
for build in "${SUCCESSFUL_BUILDS[@]}"; do
    echo -e "  ${GREEN}✓${NC} $build"
done

if [ ${#FAILED_BUILDS[@]} -gt 0 ]; then
    echo ""
    echo -e "${RED}Failed builds: ${#FAILED_BUILDS[@]}${NC}"
    for build in "${FAILED_BUILDS[@]}"; do
        echo -e "  ${RED}✗${NC} $build"
    done
    echo ""
    exit 1
else
    echo ""
    echo -e "${GREEN}All builds completed successfully!${NC}"
    echo ""
    exit 0
fi

