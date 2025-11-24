#!/bin/bash

# Build all iOS variants (Debug/Release, Static/Shared, arm64/x86_64/arm64-simulator)
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
IOS_PLATFORM_KEY="ios"
IOS_PLATFORM_DIR="$(map_platform_dir "$IOS_PLATFORM_KEY")"
SCRIPT_NAME="$(basename "$0")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=========================================="
echo "Building all iOS variants for CYLogger"
echo "=========================================="
echo ""

# Track failures
FAILED_BUILDS=()
SUCCESSFUL_BUILDS=()

# List of architecture sets to build together so build_ios.sh can emit
# universal binaries on its own. Keep in sync with build_all_platforms.sh.
declare -a IOS_ARCH_GROUPS=(
    "arm64;x86_64"
)

# Function to run a build and track results. Expects `arch_group` to be a
# semicolon-separated architecture list understood by build_ios.sh.
run_build() {
    local build_type=$1
    local lib_type=$2
    local arch_group=$3
    local arch_desc="${arch_group//;/, }"
    local description="$build_type $lib_type ($arch_desc)"
    
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo -e "${YELLOW}Building: $description${NC}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if bash "$BUILD_DIR/build_ios.sh" "$build_type" "$lib_type" "$arch_group"; then
        echo -e "${GREEN}✓ Success: $description${NC}"
        SUCCESSFUL_BUILDS+=("$description")
        return 0
    else
        echo -e "${RED}✗ Failed: $description${NC}"
        FAILED_BUILDS+=("$description")
        return 1
    fi
}

# Build each configuration once using the multi-arch build entry point.
for build_type in Debug Release; do
    for lib_type in Static Shared; do
        for arch_group in "${IOS_ARCH_GROUPS[@]}"; do
            run_build "$build_type" "$lib_type" "$arch_group"
        done
    done
done

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

