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

CC_BIN=$(detect_compiler "${CYCOROUTINE_CC:-}" "clang-17")
CXX_BIN=$(detect_compiler "${CYCOROUTINE_CXX:-}" "clang++-17")

if [ -z "$CC_BIN" ] || [ -z "$CXX_BIN" ]; then
    echo "Error: clang-17 toolchain not found (set CYCOROUTINE_CC/CYCOROUTINE_CXX to override)." >&2
    exit 1
fi

export CYCOROUTINE_CC="$CC_BIN"
export CYCOROUTINE_CXX="$CXX_BIN"

echo "Building CYCoroutine for Linux matrix..."
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
            
            if "$SCRIPT_DIR/build_linux.sh" "$build_type" "$lib_type" "$arch"; then
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
echo "All CYCoroutine Linux builds finished"
echo "========================================"

platform_dir="$(map_platform_dir linux)"
find "$OUTPUT_BASE/$platform_dir" \( -name "*.a" -o -name "*.so" -o -name "*.dylib" \) -print | sort


