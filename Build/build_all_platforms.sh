#!/bin/bash

# Multi-platform build script
# - Builds macOS/iOS slices + universal binaries
# - Produces Linux and Android static/shared libraries
# - Ensures CYCoroutine artifacts exist before compiling CYLogger

set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$PROJECT_ROOT/Bin"
IOS_TOOLCHAIN="$PROJECT_ROOT/cmake/ios.toolchain.cmake"
mkdir -p "$OUTPUT_DIR"

MACOS_DEPLOYMENT_TARGET=${MACOS_DEPLOYMENT_TARGET:-"11.0"}
IOS_DEPLOYMENT_TARGET=${IOS_DEPLOYMENT_TARGET:-"13.0"}
ANDROID_API_LEVEL_DEFAULT=${ANDROID_API_LEVEL:-"31"}

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

LINUX_ARCHES_DEFAULT=("x86_64" "x86")
ANDROID_ABIS_DEFAULT=("arm64-v8a" "armeabi-v7a" "x86_64" "x86")
BUILD_TYPES_DEFAULT=("Release" "Debug")
SHARED_KINDS_DEFAULT=("OFF" "ON")

LINUX_ARCHES=("${LINUX_ARCHES_DEFAULT[@]}")
ANDROID_ABIS=("${ANDROID_ABIS_DEFAULT[@]}")
BUILD_TYPES=("${BUILD_TYPES_DEFAULT[@]}")
SHARED_KINDS=("${SHARED_KINDS_DEFAULT[@]}")
DEFAULT_PLATFORMS=("macos" "ios" "linux" "android" "windows")
SELECTED_PLATFORMS=()
FORCE_LINUX_BUILDS=0

detect_jobs() {
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    if command -v nproc >/dev/null 2>&1; then
        nproc && return
    fi
    echo 4
}
BUILD_JOBS=$(detect_jobs)

usage() {
    cat <<'EOF'
Usage: build_all_platforms.sh [options]

Options:
  --platforms macos,ios,linux,android,windows   Comma-separated list of matrices to run.
  --build-types Release,Debug                   Comma-separated CMake build types.
  --shared-kinds static,shared                  Library flavors (static=OFF, shared=ON).
  --linux-arches x86_64,x86                     Linux architectures to compile.
  --android-abis arm64-v8a,armeabi-v7a,...      Android ABIs to compile.
  --android-api-level <level>                   Default Android API level (per-ABI mins enforced).
  --force-linux                                 Force Linux builds even on non-Linux hosts.
  -h, --help                                    Show this help and exit.

Environment overrides:
  MACOS_DEPLOYMENT_TARGET, IOS_DEPLOYMENT_TARGET, ANDROID_API_LEVEL, BUILD_JOBS.
EOF
}

map_platform_dir() {
    case "$1" in
        macos) echo "macOS" ;;
        ios) echo "iOS" ;;
        linux) echo "Linux" ;;
        windows) echo "Windows" ;;
        android) echo "Android" ;;
        *) echo "$1" ;;
    esac
}

platform_enabled() {
    local needle=$1
    local platform
    for platform in "${SELECTED_PLATFORMS[@]}"; do
        if [ "$platform" = "$needle" ]; then
            return 0
        fi
    done
    return 1
}

shared_ext_by_platform() {
    case "$1" in
        macos|ios) echo "dylib" ;;
        windows) echo "dll" ;;
        *) echo "so" ;;
    esac
}

ios_platform_token() {
    case "$1" in
        arm64) echo "OS64" ;;
        x86_64) echo "SIMULATOR64" ;;
        arm64-simulator) echo "SIMULATORARM64" ;;
        *)
            log_error "Unsupported iOS architecture: $1"
            exit 1
            ;;
    esac
}

detect_android_sdk() {
    local -a candidates=(
        "${ANDROID_SDK_ROOT:-}"
        "${ANDROID_HOME:-}"
        "${HOME}/Library/Android/sdk"
        "${HOME}/Android/Sdk"
        "${HOME}/Android/sdk"
        "/usr/local/share/android-sdk"
        "/opt/android-sdk"
    )
    for path in "${candidates[@]}"; do
        if [ -n "$path" ] && [ -d "$path" ]; then
            echo "$path"
            return
        fi
    done
}

ndk_has_toolchain() {
    local ndk_root=$1
    if [ -n "$ndk_root" ] && [ -f "$ndk_root/build/cmake/android.toolchain.cmake" ]; then
        return 0
    fi
    return 1
}

detect_android_ndk() {
    local sdk_root=$1
    local -a candidates=(
        "${ANDROID_NDK_HOME:-}"
        "${ANDROID_NDK_ROOT:-}"
        "${ANDROID_NDK:-}"
    )
    for path in "${candidates[@]}"; do
        if [ -n "$path" ] && [ -d "$path" ] && ndk_has_toolchain "$path"; then
            echo "$path"
            return
        fi
    done

    if [ -d "$sdk_root/ndk" ]; then
        local candidate
        for candidate in $(ls -1 "$sdk_root/ndk" | sort -r); do
            local full_path="$sdk_root/ndk/$candidate"
            if [ -d "$full_path" ] && ndk_has_toolchain "$full_path"; then
                echo "$full_path"
                return
            fi
        done
    fi

    if [ -d "$sdk_root/ndk-bundle" ] && ndk_has_toolchain "$sdk_root/ndk-bundle"; then
        echo "$sdk_root/ndk-bundle"
    fi
}

prepare_android_toolchain() {
    if [ "${ANDROID_SETUP_STATE:-}" = "ready" ]; then
        return 0
    fi

    local sdk_root
    sdk_root=$(detect_android_sdk)
    if [ -z "$sdk_root" ]; then
        log_error "Android SDK not found. Install it or set ANDROID_SDK_ROOT."
        exit 1
    fi

    local ndk_root
    ndk_root=$(detect_android_ndk "$sdk_root")
    if [ -z "$ndk_root" ]; then
        log_error "Android NDK not found under $sdk_root. Install it via sdkmanager."
        exit 1
    fi

    export ANDROID_SDK_ROOT="$sdk_root"
    export ANDROID_HOME="$sdk_root"
    export ANDROID_NDK_HOME="$ndk_root"
    export ANDROID_NDK_ROOT="$ndk_root"
    ANDROID_SETUP_STATE="ready"
    log_info "Detected Android SDK at $ANDROID_SDK_ROOT"
    log_info "Detected Android NDK at $ANDROID_NDK_HOME"
}

android_api_for_abi() {
    local abi=$1
    local min_api
    case "$abi" in
        armeabi-v7a|x86)
            min_api=19
            ;;
        *)
            min_api=21
            ;;
    esac

    local requested="$ANDROID_API_LEVEL_DEFAULT"
    if ! [[ $requested =~ ^[0-9]+$ ]]; then
        requested=$min_api
    fi

    if [ "$requested" -lt "$min_api" ]; then
        echo "$min_api"
    else
        echo "$requested"
    fi
}

supports_linux_arch() {
    local arch=$1
    case "$arch" in
        x86_64)
            return 0
            ;;
        x86)
            if ! command -v gcc >/dev/null 2>&1; then
                return 1
            fi
            local tmp_src tmp_out
            tmp_src=$(mktemp /tmp/cylogger32-src.XXXXXX)
            tmp_out=$(mktemp /tmp/cylogger32-bin.XXXXXX)
            echo "int main(){return 0;}" >"$tmp_src"
            if gcc -m32 "$tmp_src" -o "$tmp_out" >/dev/null 2>&1; then
                rm -f "$tmp_src" "$tmp_out"
                return 0
            fi
            rm -f "$tmp_src" "$tmp_out"
            return 1
            ;;
        *)
            return 1
            ;;
    esac
}

ensure_dependency_targets() {
    local build_dir=$1
    local platform=$2
    local arch=$3
    local build_type=$4
    local shared=$5

    local platform_dir
    platform_dir=$(map_platform_dir "$platform")
    local arch_dir="$OUTPUT_DIR/$platform_dir/$arch/$build_type"
    local static_lib="$arch_dir/libCYCoroutine.a"

    # Always build static library for CYCoroutine
    if [ ! -f "$static_lib" ]; then
        log_info "CYCoroutine static library missing, building it first..."
        cmake --build "$build_dir" --target CYCoroutine_static --config "$build_type" --parallel "$BUILD_JOBS"
    fi

    # On Windows, CYCoroutine only supports static library, so skip shared library build
    # On other platforms (macOS, Linux, Android), build shared library if requested
    if [ "$shared" = "ON" ] && [ "$platform" != "windows" ]; then
        local dyn_ext
        dyn_ext=$(shared_ext_by_platform "$platform")
        local dyn_lib="$arch_dir/libCYCoroutine.$dyn_ext"
        if [ ! -f "$dyn_lib" ]; then
            log_info "CYCoroutine shared library missing, building it first..."
            cmake --build "$build_dir" --target CYCoroutine_shared --config "$build_type" --parallel "$BUILD_JOBS"
        fi
    fi
}

build_slice() {
    local platform=$1
    local arch=$2
    local build_type=$3
    local shared=$4
    local deployment_target=${5:-}

    local shared_tag
    shared_tag=$([ "$shared" = "ON" ] && echo "shared" || echo "static")
    local build_dir="$SCRIPT_DIR/build_${platform}_${arch}_${build_type}_${shared_tag}"

    log_info "Building $platform / $arch / $build_type / $( [ "$shared" = "ON" ] && echo "shared" || echo "static")"

    local -a cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DBUILD_SHARED_LIBS=$shared"
        "-DBUILD_EXAMPLES=OFF"
        "-DTARGET_ARCH=$arch"
    )

    case "$platform" in
        macos)
            cmake_args+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$deployment_target")
            cmake_args+=("-DCMAKE_OSX_ARCHITECTURES=$arch")
            ;;
        ios)
            if [ ! -f "$IOS_TOOLCHAIN" ]; then
                log_error "Missing iOS toolchain file: $IOS_TOOLCHAIN"
                exit 1
            fi
            local ios_platform
            ios_platform=$(ios_platform_token "$arch")
            cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$IOS_TOOLCHAIN")
            cmake_args+=("-DPLATFORM=$ios_platform")
            cmake_args+=("-DARCHS=$arch")
            cmake_args+=("-DDEPLOYMENT_TARGET=$deployment_target")
            ;;
        linux)
            cmake_args+=("-DCMAKE_SYSTEM_PROCESSOR_OVERRIDE=$arch")
            if [ "$arch" = "x86" ]; then
                cmake_args+=("-DCMAKE_C_FLAGS=-m32" "-DCMAKE_CXX_FLAGS=-m32" "-DCMAKE_EXE_LINKER_FLAGS=-m32")
            elif [ "$arch" = "x86_64" ]; then
                cmake_args+=("-DCMAKE_C_FLAGS=-m64" "-DCMAKE_CXX_FLAGS=-m64")
            fi
            ;;
        android)
            prepare_android_toolchain
            cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake")
            cmake_args+=("-DANDROID_ABI=$arch")
            cmake_args+=("-DANDROID_PLATFORM=android-$deployment_target")
            cmake_args+=("-DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT")
            cmake_args+=("-DANDROID_NDK=$ANDROID_NDK_HOME")
            cmake_args+=("-DANDROID_STL=c++_static")
            ;;
        windows)
            cmake_args+=("-A" "x64")
            ;;
        *)
            log_error "Unknown platform: $platform"
            exit 1
            ;;
    esac

    cmake -S "$PROJECT_ROOT" -B "$build_dir" "${cmake_args[@]}"
    ensure_dependency_targets "$build_dir" "$platform" "$arch" "$build_type" "$shared"
    cmake --build "$build_dir" --target CYLogger --config "$build_type" --parallel "$BUILD_JOBS"

    log_info "Finished $platform / $arch / $build_type"
}

combine_universal() {
    local platform=$1
    local build_type=$2
    local shared=$3

    local platform_dir
    platform_dir=$(map_platform_dir "$platform")
    local suffix
    if [ "$shared" = "ON" ]; then
        suffix=$(shared_ext_by_platform "$platform")
    else
        suffix="a"
    fi

    if ! command -v lipo >/dev/null 2>&1; then
        log_error "lipo command not found; cannot produce universal binaries."
        exit 1
    fi

    local dest_dir="$OUTPUT_DIR/$platform_dir/universal/$build_type"
    mkdir -p "$dest_dir"

    # For iOS with arm64, x86_64, and arm64-simulator:
    # arm64-device and arm64-simulator BOTH produce arm64 architecture slices.
    # lipo -create CANNOT combine two arm64 slices (same arch conflict).
    # Solution: use ONLY arm64-device + x86_64 for universal (skip arm64-simulator).
    # arm64-simulator builds go to their own directory (not included in universal).
    local _ios_universal="false"
    if [ "$platform" = "ios" ]; then
        _ios_universal="true"
    fi

    # Determine which architectures to combine for universal.
    # On iOS: arm64 (device) + x86_64. arm64-simulator is excluded (same arch as device).
    # On macOS: arm64 + x86_64.
    local _arch1=""
    local _arch2=""
    if [ "$_ios_universal" = "true" ]; then
        _arch1="arm64"
        _arch2="x86_64"
    else
        _arch1="arm64"
        _arch2="x86_64"
    fi

    # CYLogger universal
    local logger_src_a="$OUTPUT_DIR/$platform_dir/$_arch1/$build_type/libCYLogger.$suffix"
    local logger_src_b="$OUTPUT_DIR/$platform_dir/$_arch2/$build_type/libCYLogger.$suffix"
    if [ -f "$logger_src_a" ] && [ -f "$logger_src_b" ]; then
        log_info "Creating CYLogger $platform_dir $build_type universal binary (arch: $_arch1 + $_arch2)"
        lipo -create "$logger_src_a" "$logger_src_b" -output "$dest_dir/libCYLogger.$suffix"
    else
        log_warn "Missing CYLogger universal inputs. Skipping."
    fi

    # CYCoroutine universal
    local coroutine_src_a="$OUTPUT_DIR/$platform_dir/$_arch1/$build_type/libCYCoroutine.$suffix"
    local coroutine_src_b="$OUTPUT_DIR/$platform_dir/$_arch2/$build_type/libCYCoroutine.$suffix"
    if [ -f "$coroutine_src_a" ] && [ -f "$coroutine_src_b" ]; then
        log_info "Creating CYCoroutine $platform_dir $build_type universal binary (arch: $_arch1 + $_arch2)"
        lipo -create "$coroutine_src_a" "$coroutine_src_b" -output "$dest_dir/libCYCoroutine.$suffix"
    else
        log_warn "Missing CYCoroutine universal inputs. Skipping."
    fi
}

build_macos_matrix() {
    local deployment_target=$1
    for build_type in "${BUILD_TYPES[@]}"; do
        for shared in "${SHARED_KINDS[@]}"; do
            build_slice "macos" "arm64" "$build_type" "$shared" "$deployment_target"
            build_slice "macos" "x86_64" "$build_type" "$shared" "$deployment_target"
            combine_universal "macos" "$build_type" "$shared"
        done
    done
}

build_ios_matrix() {
    local deployment_target=$1
    # CYLogger's build_ios.sh handles all iOS build logic correctly:
    # - All three architectures: arm64 (device), x86_64, arm64-simulator
    # - Both Static and Shared library types
    # - Universal binary creation with proper arm64-device/arm64-simulator conflict avoidance
    # - CYCommon and CYCoroutine dependency management
    # It is invoked once per build type with the full arch list.
    # NOTE: build_ios.sh is called instead of build_slice because:
    #   1. build_slice -> CYLogger CMakeLists.txt directly misses CYCommon/CYCoroutine dependency
    #   2. build_ios.sh properly integrates CYCommon and CYCoroutine builds via add_subdirectory
    #   3. build_ios.sh handles arm64-simulator lipo conflicts (arm64-device vs arm64-simulator)
    #   4. build_ios.sh matches the existing build_all_IOS.sh usage pattern
    for build_type in "${BUILD_TYPES[@]}"; do
        for shared in "${SHARED_KINDS[@]}"; do
            local lib_type
            if [ "$shared" = "ON" ]; then
                lib_type="Shared"
            else
                lib_type="Static"
            fi
            local arch_group="arm64;x86_64;arm64-simulator"
            log_info "Building iOS: $build_type $lib_type ($arch_group)"
            if bash "$SCRIPT_DIR/build_ios.sh" "$build_type" "$lib_type" "$arch_group"; then
                log_info "Success: iOS $build_type $lib_type"
            else
                log_error "Failed: iOS $build_type $lib_type"
                exit 1
            fi
        done
    done
}

build_linux_matrix() {
    for build_type in "${BUILD_TYPES[@]}"; do
        for shared in "${SHARED_KINDS[@]}"; do
            for arch in "${LINUX_ARCHES[@]}"; do
                if supports_linux_arch "$arch"; then
                    build_slice "linux" "$arch" "$build_type" "$shared"
                else
                    log_warn "Skipping Linux $arch build. Required toolchain or multilib support is missing."
                fi
            done
        done
    done
}

build_android_matrix() {
    prepare_android_toolchain
    for build_type in "${BUILD_TYPES[@]}"; do
        for shared in "${SHARED_KINDS[@]}"; do
            for abi in "${ANDROID_ABIS[@]}"; do
                local api_level
                api_level=$(android_api_for_abi "$abi")
                build_slice "android" "$abi" "$build_type" "$shared" "$api_level"
            done
        done
    done
}

build_windows_matrix() {
    for build_type in "${BUILD_TYPES[@]}"; do
        for shared in "${SHARED_KINDS[@]}"; do
            build_slice "windows" "x64" "$build_type" "$shared"
        done
    done
}

parse_cli_args() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --platforms)
                if [[ $# -lt 2 ]]; then
                    log_error "--platforms requires a value"
                    usage
                    exit 1
                fi
                local value=$2
                local -a raw
                local IFS=','
                read -r -a raw <<< "$value"
                SELECTED_PLATFORMS=()
                local token
                for token in "${raw[@]}"; do
                    token=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
                    token=${token//[[:space:]]/}
                    if [ -n "$token" ]; then
                        SELECTED_PLATFORMS+=("$token")
                    fi
                done
                shift 2
                ;;
            --build-types)
                if [[ $# -lt 2 ]]; then
                    log_error "--build-types requires a value"
                    usage
                    exit 1
                fi
                local value=$2
                local -a raw
                local IFS=','
                read -r -a raw <<< "$value"
                BUILD_TYPES=()
                local token
                for token in "${raw[@]}"; do
                    token=$(printf '%s' "$token" | tr -d '[:space:]')
                    if [ -n "$token" ]; then
                        BUILD_TYPES+=("$token")
                    fi
                done
                shift 2
                ;;
            --shared-kinds)
                if [[ $# -lt 2 ]]; then
                    log_error "--shared-kinds requires a value"
                    usage
                    exit 1
                fi
                local value=$2
                local -a raw
                local IFS=','
                read -r -a raw <<< "$value"
                SHARED_KINDS=()
                local token
                for token in "${raw[@]}"; do
                    token=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
                    token=${token//[[:space:]]/}
                    case "$token" in
                        on|shared)
                            SHARED_KINDS+=("ON")
                            ;;
                        off|static)
                            SHARED_KINDS+=("OFF")
                            ;;
                        "")
                            ;;
                        *)
                            log_warn "Ignoring unknown shared kind: $token"
                            ;;
                    esac
                done
                shift 2
                ;;
            --linux-arches)
                if [[ $# -lt 2 ]]; then
                    log_error "--linux-arches requires a value"
                    usage
                    exit 1
                fi
                local value=$2
                local -a raw
                local IFS=','
                read -r -a raw <<< "$value"
                LINUX_ARCHES=()
                local token
                for token in "${raw[@]}"; do
                    token=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
                    token=${token//[[:space:]]/}
                    if [ -n "$token" ]; then
                        LINUX_ARCHES+=("$token")
                    fi
                done
                shift 2
                ;;
            --android-abis)
                if [[ $# -lt 2 ]]; then
                    log_error "--android-abis requires a value"
                    usage
                    exit 1
                fi
                local value=$2
                local -a raw
                local IFS=','
                read -r -a raw <<< "$value"
                ANDROID_ABIS=()
                local token
                for token in "${raw[@]}"; do
                    token=$(printf '%s' "$token" | tr '[:upper:]' '[:lower:]')
                    token=${token//[[:space:]]/}
                    if [ -n "$token" ]; then
                        ANDROID_ABIS+=("$token")
                    fi
                done
                shift 2
                ;;
            --android-api-level)
                if [[ $# -lt 2 ]]; then
                    log_error "--android-api-level requires a numeric value"
                    usage
                    exit 1
                fi
                ANDROID_API_LEVEL_DEFAULT=$2
                shift 2
                ;;
            --force-linux)
                FORCE_LINUX_BUILDS=1
                shift 1
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                log_error "Unknown argument: $1"
                usage
                exit 1
                ;;
        esac
    done

    if [ ${#SELECTED_PLATFORMS[@]} -eq 0 ]; then
        SELECTED_PLATFORMS=("${DEFAULT_PLATFORMS[@]}")
    fi
    if [ ${#BUILD_TYPES[@]} -eq 0 ]; then
        BUILD_TYPES=("${BUILD_TYPES_DEFAULT[@]}")
    fi
    if [ ${#SHARED_KINDS[@]} -eq 0 ]; then
        SHARED_KINDS=("${SHARED_KINDS_DEFAULT[@]}")
    fi
    if [ ${#LINUX_ARCHES[@]} -eq 0 ]; then
        LINUX_ARCHES=("${LINUX_ARCHES_DEFAULT[@]}")
    fi
    if [ ${#ANDROID_ABIS[@]} -eq 0 ]; then
        ANDROID_ABIS=("${ANDROID_ABIS_DEFAULT[@]}")
    fi
}

main() {
    parse_cli_args "$@"
    log_info "Starting CYLogger multi-platform build"
    log_info "Enabled platforms: ${SELECTED_PLATFORMS[*]}"
    log_info "Build types: ${BUILD_TYPES[*]}"
    log_info "Library flavors: ${SHARED_KINDS[*]}"

    if platform_enabled "macos"; then
        log_info "=== macOS matrix ==="
        build_macos_matrix "$MACOS_DEPLOYMENT_TARGET"
    fi

    if platform_enabled "ios"; then
        log_info "=== iOS matrix ==="
        build_ios_matrix "$IOS_DEPLOYMENT_TARGET"
    fi

    if platform_enabled "linux"; then
        if [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "linux"* ]] || [ "$FORCE_LINUX_BUILDS" -eq 1 ]; then
            log_info "=== Linux matrix ==="
            build_linux_matrix
        else
            log_warn "Linux builds require a Linux host or --force-linux with a cross toolchain. Skipping."
        fi
    fi

    if platform_enabled "android"; then
        log_info "=== Android matrix ==="
        build_android_matrix
    fi

    if platform_enabled "windows"; then
        if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "win32" ]]; then
            log_info "=== Windows matrix ==="
            build_windows_matrix
        else
            log_warn "Windows builds must run from MSYS/MinGW shells. Skipping."
        fi
    fi

    log_info "Build artifacts are available under $OUTPUT_DIR"
}

main "$@"