#!/bin/bash

# Build every Android ABI in one go
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}
ANDROID_API_LEVEL=${3:-31}

echo "Building CYLogger for all Android ABIs..."
echo "Build Type: $BUILD_TYPE"
echo "Library Type: $LIB_TYPE"
echo "Android API Level: $ANDROID_API_LEVEL (will be adjusted per ABI if needed)"

ANDROID_ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")

for ABI in "${ANDROID_ABIS[@]}"; do
    echo ""
    echo "========================================"
    echo "Building for Android ABI: $ABI"
    echo "========================================"

    if "$BUILD_DIR/build_android.sh" "$BUILD_TYPE" "$LIB_TYPE" "$ABI" "$ANDROID_API_LEVEL"; then
        echo "Successfully built for $ABI"
    else
        echo "Failed to build for $ABI"
        exit 1
    fi
done

echo ""
echo "========================================"
echo "All Android builds completed successfully!"
echo "========================================"

echo ""
echo "Generated libraries:"
find "$PROJECT_ROOT/Bin/Android" \( -name "*.so" -o -name "*.a" \) -print | sort