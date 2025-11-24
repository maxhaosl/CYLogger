#!/bin/bash

# Build matrix for all Linux architectures / build types / library kinds
set -euo pipefail

BUILD_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$BUILD_DIR/.."

BUILD_TYPES_CSV=${1:-"Release,Debug"}
LIB_TYPES_CSV=${2:-"Static,Shared"}
ARCHES_CSV=${3:-"x86_64,x86"}

IFS=',' read -ra BUILD_TYPES <<<"$BUILD_TYPES_CSV"
IFS=',' read -ra LIB_TYPES <<<"$LIB_TYPES_CSV"
IFS=',' read -ra LINUX_ARCHES <<<"$ARCHES_CSV"

trim_array() {
    local -n arr=$1
    for i in "${!arr[@]}"; do
        arr[$i]="$(echo "${arr[$i]}" | xargs)"
    done
}

trim_array BUILD_TYPES
trim_array LIB_TYPES
trim_array LINUX_ARCHES

detect_jobs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc && return
    fi
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 4
}

ensure_cycoroutine_linux() {
    local arch=$1
    local build_type=$2
    local lib_type=$3

    local arch_dir="$PROJECT_ROOT/Bin/Linux/$arch/$build_type"
    local static_lib="$arch_dir/libCYCoroutine.a"
    local shared_lib="$arch_dir/libCYCoroutine.so"

    local need_static=0
    local need_shared=0

    if [ ! -f "$static_lib" ]; then
        need_static=1
    fi

    if [ "$lib_type" = "Shared" ] && [ ! -f "$shared_lib" ]; then
        need_shared=1
    fi

    if [ "$need_static" -eq 0 ] && [ "$need_shared" -eq 0 ]; then
        echo "CYCoroutine dependencies already present for ${arch}/${build_type}/${lib_type}"
        return
    fi

    local shared_flag
    if [ "$lib_type" = "Shared" ]; then
        shared_flag="ON"
    else
        shared_flag="OFF"
    fi

    local dep_dir="$BUILD_DIR/deps_linux_${arch}_${build_type}_${shared_flag}"
    local -a cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DBUILD_SHARED_LIBS=$shared_flag"
        "-DBUILD_STATIC_LIBS=ON"
        "-DBUILD_EXAMPLES=OFF"
        "-DUSE_CYCOROUTINE=ON"
        "-DCYLOGGER_ROOT_DIR=$PROJECT_ROOT"
        "-DTARGET_ARCH=$arch"
        "-DCMAKE_SYSTEM_PROCESSOR_OVERRIDE=$arch"
    )

    if [ "$arch" = "x86" ]; then
        cmake_args+=("-DCMAKE_C_FLAGS=-m32" "-DCMAKE_CXX_FLAGS=-m32" "-DCMAKE_EXE_LINKER_FLAGS=-m32")
    elif [ "$arch" = "x86_64" ]; then
        cmake_args+=("-DCMAKE_C_FLAGS=-m64" "-DCMAKE_CXX_FLAGS=-m64")
    fi

    echo "Preparing CYCoroutine dependency for ${arch}/${build_type}/${lib_type}..."
    cmake -S "$PROJECT_ROOT" -B "$dep_dir" "${cmake_args[@]}"

    if [ "$need_static" -eq 1 ]; then
        echo "Building CYCoroutine_static for ${arch}/${build_type}..."
        cmake --build "$dep_dir" --target CYCoroutine_static --parallel "$(detect_jobs)"
    fi

    if [ "$need_shared" -eq 1 ]; then
        echo "Building CYCoroutine_shared for ${arch}/${build_type}..."
        cmake --build "$dep_dir" --target CYCoroutine_shared --parallel "$(detect_jobs)"
    fi
}

echo "Building CYLogger for all Linux slices..."
echo "Build types: ${BUILD_TYPES[*]}"
echo "Library kinds: ${LIB_TYPES[*]}"
echo "Architectures: ${LINUX_ARCHES[*]}"

for arch in "${LINUX_ARCHES[@]}"; do
    for build_type in "${BUILD_TYPES[@]}"; do
        for lib_type in "${LIB_TYPES[@]}"; do
            echo ""
            echo "========================================"
            echo "Arch: ${arch} | Build: ${build_type} | Lib: ${lib_type}"
            echo "========================================"
            ensure_cycoroutine_linux "$arch" "$build_type" "$lib_type"
            if "$BUILD_DIR/build_linux.sh" "$build_type" "$lib_type" "$arch"; then
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
echo "All Linux builds finished successfully"
echo "========================================"

echo ""
echo "Generated libraries:"
find "$PROJECT_ROOT/Bin/Linux" \( -name "*.a" -o -name "*.so" \) -print | sort


