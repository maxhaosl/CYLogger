#!/bin/bash

# Universal build script - detects host OS and runs the proper helper
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$BUILD_DIR/output_layout.sh"
HOST_PLATFORM_KEY=""
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}

echo "CYLogger Universal Build Script"
echo "==============================="

# Detect the host operating system
OS=$(uname -s)
case "$OS" in
    Darwin*)
        echo "Detected macOS"
        if [ -f "$BUILD_DIR/build_macos.sh" ]; then
            chmod +x "$BUILD_DIR/build_macos.sh"
            "$BUILD_DIR/build_macos.sh" "$BUILD_TYPE" "$LIB_TYPE"
            HOST_PLATFORM_KEY="macos"
        else
            echo "Error: build_macos.sh not found!"
            exit 1
        fi
        ;;
    Linux*)
        echo "Detected Linux"
        if [ -f "$BUILD_DIR/build_linux.sh" ]; then
            chmod +x "$BUILD_DIR/build_linux.sh"
            "$BUILD_DIR/build_linux.sh" "$BUILD_TYPE" "$LIB_TYPE"
            HOST_PLATFORM_KEY="linux"
        else
            echo "Error: build_linux.sh not found!"
            exit 1
        fi
        ;;
    CYGWIN*|MINGW*|MSYS*)
        echo "Detected Windows"
        if [ -f "$BUILD_DIR/build_windows.bat" ]; then
            "$BUILD_DIR/build_windows.bat" "$BUILD_TYPE" "$LIB_TYPE"
            HOST_PLATFORM_KEY="windows"
        else
            echo "Error: build_windows.bat not found!"
            exit 1
        fi
        ;;
    *)
        echo "Unsupported operating system: $OS"
        echo "Please use the platform-specific build script:"
        echo "  - Windows: build_windows.bat"
        echo "  - macOS: build_macos.sh"
        echo "  - Linux: build_linux.sh"
        echo "  - Android: build_android.sh"
        echo "  - iOS: build_ios.sh"
        exit 1
        ;;
esac

echo ""
echo "Build completed successfully!"
if [ -n "$HOST_PLATFORM_KEY" ]; then
    HOST_PLATFORM_DIR="$(map_platform_dir "$HOST_PLATFORM_KEY")"
    echo "Check the output under: $OUTPUT_BASE/$HOST_PLATFORM_DIR"
else
    echo "Check the output in: $OUTPUT_BASE/"
fi