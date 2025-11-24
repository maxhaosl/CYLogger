#!/bin/bash

# Shared helpers for keeping platform-specific output directories consistent with
# build_all_platforms.sh. All scripts that source this file must define
# OUTPUT_BASE before sourcing.

map_platform_dir() {
    local raw="$1"
    local normalized
    normalized=$(printf '%s' "$raw" | tr '[:upper:]' '[:lower:]')
    case "$normalized" in
        macos|mac|darwin)
            echo "macOS"
            ;;
        ios|iphone|ipad)
            echo "iOS"
            ;;
        linux|gnu-linux)
            echo "Linux"
            ;;
        android)
            echo "Android"
            ;;
        windows|win|msys|mingw|cygwin)
            echo "Windows"
            ;;
        *)
            # Fall back to the provided token so custom directories keep working.
            echo "$raw"
            ;;
    esac
}

platform_slice_dir() {
    local platform=$1
    local arch=$2
    local build_type=$3
    local platform_dir
    platform_dir=$(map_platform_dir "$platform")
    echo "$OUTPUT_BASE/$platform_dir/$arch/$build_type"
}

platform_universal_dir() {
    local platform=$1
    local build_type=$2
    local platform_dir
    platform_dir=$(map_platform_dir "$platform")
    echo "$OUTPUT_BASE/$platform_dir/universal/$build_type"
}

