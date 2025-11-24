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
    
    local dest_dir
    dest_dir="$(platform_universal_dir "$MACOS_PLATFORM_KEY" "$build_type")"
    mkdir -p "$dest_dir"
    
    # Combine CYLogger universal library
    local logger_src_arm
    logger_src_arm="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYLogger.$suffix"
    local logger_src_x86
    logger_src_x86="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYLogger.$suffix"
    local logger_dest="$dest_dir/libCYLogger.$suffix"
    
    if [ -f "$logger_src_arm" ] && [ -f "$logger_src_x86" ]; then
        echo -e "${YELLOW}Creating CYLogger $build_type $lib_type universal binary...${NC}"
        lipo -create "$logger_src_arm" "$logger_src_x86" -output "$logger_dest"
        echo -e "${GREEN}✓ Created: $logger_dest${NC}"
    else
        echo -e "${RED}Warning: Missing CYLogger $build_type $lib_type inputs for lipo.${NC}"
        [ ! -f "$logger_src_arm" ] && echo -e "  Missing: $logger_src_arm"
        [ ! -f "$logger_src_x86" ] && echo -e "  Missing: $logger_src_x86"
    fi
    
    # Combine CYCoroutine universal library
    local coroutine_src_arm
    coroutine_src_arm="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "arm64" "$build_type")/libCYCoroutine.$suffix"
    local coroutine_src_x86
    coroutine_src_x86="$(platform_slice_dir "$MACOS_PLATFORM_KEY" "x86_64" "$build_type")/libCYCoroutine.$suffix"
    local coroutine_dest="$dest_dir/libCYCoroutine.$suffix"
    
    if [ -f "$coroutine_src_arm" ] && [ -f "$coroutine_src_x86" ]; then
        echo -e "${YELLOW}Creating CYCoroutine $build_type $lib_type universal binary...${NC}"
        lipo -create "$coroutine_src_arm" "$coroutine_src_x86" -output "$coroutine_dest"
        echo -e "${GREEN}✓ Created: $coroutine_dest${NC}"
    else
        echo -e "${RED}Warning: Missing CYCoroutine $build_type $lib_type inputs for lipo.${NC}"
        [ ! -f "$coroutine_src_arm" ] && echo -e "  Missing: $coroutine_src_arm"
        [ ! -f "$coroutine_src_x86" ] && echo -e "  Missing: $coroutine_src_x86"
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

# Also build universal variants directly (CMake can handle it)
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Building universal variants directly...${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

run_build "Debug" "Static" "x86_64;arm64"
run_build "Debug" "Shared" "x86_64;arm64"
run_build "Release" "Static" "x86_64;arm64"
run_build "Release" "Shared" "x86_64;arm64"

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

