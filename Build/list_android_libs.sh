#!/bin/bash

# List all generated Android libraries
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."

echo "========================================"
echo "CYLogger Android Libraries"
echo "========================================"
echo ""

# Enumerate static libraries
echo "Static Libraries (.a):"
find "$PROJECT_ROOT/Bin/Android" -name "*.a" | sort | while read file; do
    size=$(ls -lh "$file" | awk '{print $5}')
    echo "  $file ($size)"
done

echo ""
echo "Shared Libraries (.so):"
find "$PROJECT_ROOT/Bin/Android" -name "*.so" | sort | while read file; do
    size=$(ls -lh "$file" | awk '{print $5}')
    echo "  $file ($size)"
done

echo ""
echo "========================================"
echo "Total Libraries:"
echo "Static: $(find "$PROJECT_ROOT/Bin/Android" -name "*.a" | wc -l | tr -d ' ')"
echo "Shared: $(find "$PROJECT_ROOT/Bin/Android" -name "*.so" | wc -l | tr -d ' ')"
echo "========================================"