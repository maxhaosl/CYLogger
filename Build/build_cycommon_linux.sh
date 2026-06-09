#!/bin/bash
# Build CYCommon static library into CYLogger/Bin/Linux/<arch>/<config>/
# CYCommon itself outputs to its own Bin/Linux/<arch>/<build_type>/ directory.
# This script copies it to CYLogger's Bin/Linux/<arch>/<build_type>/ for consistency.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_BASE="$PROJECT_ROOT/Bin"
source "$SCRIPT_DIR/output_layout.sh"

BUILD_TYPE="${1:-Release}"
RAW_ARCH="${2:-x86_64}"

canonicalize_linux_arch() {
    local token lower
    token=$(printf '%s' "$1" | xargs)
    lower=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
    case "$lower" in
        x86_64|amd64|x64) echo "x86_64" ;;
        i386|i686|x86) echo "x86" ;;
        arm64|aarch64) echo "arm64" ;;
        *) echo "$token" ;;
    esac
}

case "$BUILD_TYPE" in
    d|D|debug) BUILD_TYPE="Debug" ;;
    r|R|release) BUILD_TYPE="Release" ;;
esac

TARGET_ARCH="$(canonicalize_linux_arch "$RAW_ARCH")"
OUTPUT_SLICE="$(platform_slice_dir linux "$TARGET_ARCH" "$BUILD_TYPE")"

if [ "$BUILD_TYPE" = "Debug" ]; then
    LIB_NAME="libCYCommonD.a"
else
    LIB_NAME="libCYCommon.a"
fi

# CYCommon outputs to its own Bin/Linux/<arch>/<build_type>/ directory
CYCOMMON_OUTPUT="$PROJECT_ROOT/ThirdParty/CYCommon/Bin/Linux/$TARGET_ARCH/$BUILD_TYPE/$LIB_NAME"

# Check if already present in CYLogger's output slice
if [ -f "$OUTPUT_SLICE/$LIB_NAME" ]; then
    echo "CYCommon already present: $OUTPUT_SLICE/$LIB_NAME"
    exit 0
fi

# Check if CYCommon has already been built in its own directory
if [ ! -f "$CYCOMMON_OUTPUT" ]; then
    CYCOMMON_BUILD="$PROJECT_ROOT/ThirdParty/CYCommon/Build/build_linux.sh"
    if [ ! -f "$CYCOMMON_BUILD" ]; then
        echo "ERROR: CYCommon build script not found: $CYCOMMON_BUILD" >&2
        exit 1
    fi
    echo "Building CYCommon (outputs to ThirdParty/CYCommon/Bin/)"
    bash "$CYCOMMON_BUILD" "$BUILD_TYPE" "$TARGET_ARCH"
fi

if [ ! -f "$CYCOMMON_OUTPUT" ]; then
    echo "ERROR: Expected $CYCOMMON_OUTPUT after CYCommon build" >&2
    exit 1
fi

# Copy to CYLogger's output slice
mkdir -p "$OUTPUT_SLICE"
cp "$CYCOMMON_OUTPUT" "$OUTPUT_SLICE/$LIB_NAME"
echo "CYCommon staged: $OUTPUT_SLICE/$LIB_NAME"
