#!/bin/bash

# 通用构建脚本 - 自动检测平台并构建
set -e

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."
BUILD_TYPE=${1:-Release}
LIB_TYPE=${2:-Static}

echo "CYLogger Universal Build Script"
echo "==============================="

# 检测操作系统
OS=$(uname -s)
case "$OS" in
    Darwin*)
        echo "Detected macOS"
        if [ -f "$BUILD_DIR/build_macos.sh" ]; then
            chmod +x "$BUILD_DIR/build_macos.sh"
            "$BUILD_DIR/build_macos.sh" "$BUILD_TYPE" "$LIB_TYPE"
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
        else
            echo "Error: build_linux.sh not found!"
            exit 1
        fi
        ;;
    CYGWIN*|MINGW*|MSYS*)
        echo "Detected Windows"
        if [ -f "$BUILD_DIR/build_windows.bat" ]; then
            "$BUILD_DIR/build_windows.bat" "$BUILD_TYPE" "$LIB_TYPE"
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
echo "Check the output in: $PROJECT_ROOT/Bin/"