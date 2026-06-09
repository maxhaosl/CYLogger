#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_BASE="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_BASE"
source "$SCRIPT_DIR/output_layout.sh"

# Fixed configuration for all build types, library types and architectures
BUILD_TYPES_CSV="Release,Debug"
LIB_TYPES_CSV="Static,Shared"
ARCHES_CSV="x86_64,x86"

# Ensure CYCommon submodule exists
ensure_cycommon_submodule() {
    # CYCommon can be in multiple locations:
    # 1. ThirdParty/CYCommon (bundled with CYLogger)
    # 2. ../../../CYCommon (sibling location)
    local cycommon_header="$PROJECT_ROOT/ThirdParty/CYCommon/Inc/CYCommon/CYCommon.hpp"
    local cycommon_build="$PROJECT_ROOT/ThirdParty/CYCommon/Build/CMakeLists.txt"

    if [ -f "$cycommon_header" ] && [ -f "$cycommon_build" ]; then
        return 0
    fi

    # Check sibling location
    local cycommon_sibling="$PROJECT_ROOT/../../../CYCommon/Inc/CYCommon/CYCommon.hpp"
    if [ -f "$cycommon_sibling" ]; then
        return 0
    fi

    echo "CYCommon not found at expected locations:"
    echo "  - $cycommon_header"
    echo "  - $cycommon_sibling"
    echo "Please ensure CYCommon is available as a local dependency."
    return 1
}

ensure_cycommon_submodule || exit 1

canonicalize_linux_arch() {
    local token lower
    token=$(printf '%s' "$1" | xargs)
    lower=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
    case "$lower" in
        x86_64|amd64|x64)
            echo "x86_64"
            ;;
        i386|i686|x86)
            echo "x86"
            ;;
        arm64|aarch64)
            echo "arm64"
            ;;
        *)
            echo "$token"
            ;;
    esac
}

detect_compiler() {
    local override="$1" required="$2" resolved
    if [ -n "$override" ]; then
        echo "$override"
        return
    fi
    if command -v "$required" >/dev/null 2>&1; then
        resolved=$(command -v "$required")
        echo "$resolved"
        return
    fi
    echo ""
}

IFS=',' read -ra BUILD_TYPES <<<"$BUILD_TYPES_CSV"
IFS=',' read -ra LIB_TYPES <<<"$LIB_TYPES_CSV"
IFS=',' read -ra ARCH_LIST <<<"$ARCHES_CSV"

trim_array() {
    local -n arr=$1
    local i
    for i in "${!arr[@]}"; do
        arr[$i]="$(echo "${arr[$i]}" | xargs)"
    done
}

trim_array BUILD_TYPES
trim_array LIB_TYPES
trim_array ARCH_LIST

for i in "${!ARCH_LIST[@]}"; do
    ARCH_LIST[$i]="$(canonicalize_linux_arch "${ARCH_LIST[$i]}")"
done

echo "Building CYLogger for Linux matrix..."
echo "Build types: ${BUILD_TYPES[*]}"
echo "Library kinds: ${LIB_TYPES[*]}"
echo "Architectures: ${ARCH_LIST[*]}"

for arch in "${ARCH_LIST[@]}"; do
    for build_type in "${BUILD_TYPES[@]}"; do
        for lib_type in "${LIB_TYPES[@]}"; do
            [ -z "$arch" ] && continue
            [ -z "$build_type" ] && continue
            [ -z "$lib_type" ] && continue

            echo ""
            echo "========================================"
            echo "Arch: $arch | Build: $build_type | Lib: $lib_type"
            echo "========================================"
            
            # Skip x86 builds as they don't support C++20 coroutines with clang-17
            if [ "$arch" = "x86" ]; then
                echo "⚠ Skipping x86 builds - C++20 coroutines not supported with clang-17"
                continue
            fi

            # Build CYCommon once per arch/config (output: Bin/Linux/<arch>/<build_type>/)
            if ! bash "$SCRIPT_DIR/build_cycommon_linux.sh" "$build_type" "$arch"; then
                echo "✖ CYCommon build failed for ${arch}/${build_type}"
                exit 1
            fi

            # Pass CYLOGGER_WANT_SHARED for shared builds so CYLogger_shared target is built
            if [ "$lib_type" = "Shared" ]; then
                export CYLOGGER_WANT_SHARED=ON
            else
                export CYLOGGER_WANT_SHARED=OFF
            fi

            if bash "$SCRIPT_DIR/build_linux.sh" "$build_type" "$lib_type" "$arch"; then
                echo "✔ Success for ${arch}/${build_type}/${lib_type}"
            else
                echo "✖ Failed for ${arch}/${build_type}/${lib_type}"
                exit 1
            fi
        done
    done
done

echo ""
echo "========================================"
echo "All CYLogger Linux builds finished"
echo "========================================"

platform_dir="$(map_platform_dir linux)"
find "$OUTPUT_BASE/$platform_dir" \( -name "*.a" -o -name "*.so" -o -name "*.dylib" \) -print | sort
echo ""
echo "CYCommon static libraries under Bin/Linux:"
find "$OUTPUT_BASE/$platform_dir" \( -name "libCYCommon.a" -o -name "libCYCommonD.a" \) -print | sort


