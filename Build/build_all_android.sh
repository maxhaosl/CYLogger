#!/bin/bash

# Build every Android ABI in one go (all BuildType combinations)
# CYLogger itself builds both static and shared in one call.
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
ANDROID_PLATFORM_DIR="$(map_platform_dir android)"
ANDROID_API_LEVEL=${1:-31}

echo "Building CYLogger for all Android ABIs..."
echo "Android API Level: $ANDROID_API_LEVEL (will be adjusted per ABI if needed)"
echo ""

BUILD_TYPES=("Debug" "Release")
ANDROID_ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")

failed=0
for BUILD_TYPE in "${BUILD_TYPES[@]}"; do
    for ABI in "${ANDROID_ABIS[@]}"; do
        echo "========================================"
        echo "Building for Android ABI: $ABI"
        echo "  Build Type : $BUILD_TYPE"
        echo "========================================"

        # build_android.sh now handles both static and shared in one call
        if "$BUILD_DIR/build_android.sh" "$BUILD_TYPE" "$ABI" "$ANDROID_API_LEVEL"; then
            echo "Successfully built for $ABI ($BUILD_TYPE)"
        else
            echo "Failed to build for $ABI ($BUILD_TYPE)"
            failed=1
        fi
        echo ""
    done
done

echo ""
echo "========================================"
if [ "$failed" -eq 0 ]; then
    echo "All Android builds completed successfully!"
else
    echo "Some Android builds FAILED!"
    exit 1
fi
echo "========================================"

echo ""
echo "Generated libraries:"
find "$OUTPUT_BASE/$ANDROID_PLATFORM_DIR" \( -name "*.so" -o -name "*.a" \) -print | sort
