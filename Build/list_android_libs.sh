#!/bin/bash

# List all generated Android libraries
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
ANDROID_PLATFORM_DIR="$(map_platform_dir android)"
ANDROID_ROOT="$OUTPUT_BASE/$ANDROID_PLATFORM_DIR"

echo "========================================"
echo "CYLogger Android Libraries"
echo "========================================"
echo ""

# Enumerate static libraries
echo "Static Libraries (.a):"
find "$ANDROID_ROOT" -name "*.a" | sort | while read file; do
    size=$(ls -lh "$file" | awk '{print $5}')
    echo "  $file ($size)"
done

echo ""
echo "Shared Libraries (.so):"
find "$ANDROID_ROOT" -name "*.so" | sort | while read file; do
    size=$(ls -lh "$file" | awk '{print $5}')
    echo "  $file ($size)"
done

echo ""
echo "========================================"
echo "Total Libraries:"
echo "Static: $(find "$ANDROID_ROOT" -name "*.a" | wc -l | tr -d ' ')"
echo "Shared: $(find "$ANDROID_ROOT" -name "*.so" | wc -l | tr -d ' ')"
echo "========================================"