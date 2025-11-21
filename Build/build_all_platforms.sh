#!/bin/bash

# 跨平台构建脚本
# 编译各个平台的静态库和动态库
# Mac/iOS的不同处理器库合并为一个通用库

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 创建输出目录
OUTPUT_DIR="$PROJECT_ROOT/Bin"
mkdir -p "$OUTPUT_DIR"

# 构建函数
build_platform() {
    local platform=$1
    local arch=$2
    local build_type=$3
    local shared=$4
    local deployment_target=$5
    
    local build_dir="$SCRIPT_DIR/build_${platform}_${arch}_${build_type}"
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    log_info "开始构建 $platform $arch $build_type ${shared:+动态库} ${shared:-静态库}"
    
    # 设置CMake参数
    local cmake_args=(
        "-DCMAKE_BUILD_TYPE=$build_type"
        "-DCMAKE_CXX_STANDARD=20"
        "-DBUILD_SHARED_LIBS=$shared"
    )
    
    # 平台特定参数
    case $platform in
        "macos")
            cmake_args+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$deployment_target")
            if [ "$arch" = "universal" ]; then
                # 构建通用库需要分别构建不同架构然后合并
                build_macos_universal "$build_type" "$shared" "$deployment_target"
                return $?
            else
                cmake_args+=("-DCMAKE_OSX_ARCHITECTURES=$arch")
            fi
            ;;
        "ios")
            cmake_args+=("-DCMAKE_SYSTEM_NAME=iOS")
            cmake_args+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=$deployment_target")
            if [ "$arch" = "universal" ]; then
                # 构建通用库需要分别构建不同架构然后合并
                build_ios_universal "$build_type" "$shared" "$deployment_target"
                return $?
            else
                cmake_args+=("-DCMAKE_OSX_ARCHITECTURES=$arch")
            fi
            ;;
        "linux")
            # Linux特定设置
            ;;
        "windows")
            # Windows特定设置
            ;;
    esac
    
    # 配置和构建
    cmake "$PROJECT_ROOT" "${cmake_args[@]}"
    make -j$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)
    
    log_info "完成构建 $platform $arch $build_type ${shared:+动态库} ${shared:-静态库}"
}

# macOS通用库构建函数
build_macos_universal() {
    local build_type=$1
    local shared=$2
    local deployment_target=$3
    
    log_info "构建macOS通用库 $build_type ${shared:+动态库} ${shared:-静态库}"
    
    # 分别构建x86_64和arm64
    local archs=("x86_64" "arm64")
    local build_dirs=()
    
    for arch in "${archs[@]}"; do
        local arch_build_dir="$SCRIPT_DIR/build_macos_${arch}_${build_type}"
        mkdir -p "$arch_build_dir"
        cd "$arch_build_dir"
        
        log_info "构建macOS $arch $build_type ${shared:+动态库} ${shared:-静态库}"
        
        cmake "$PROJECT_ROOT" \
            "-DCMAKE_BUILD_TYPE=$build_type" \
            "-DCMAKE_CXX_STANDARD=20" \
            "-DBUILD_SHARED_LIBS=$shared" \
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=$deployment_target" \
            "-DCMAKE_OSX_ARCHITECTURES=$arch"
        
        make -j$(sysctl -n hw.ncpu)
        build_dirs+=("$arch_build_dir")
    done
    
    # 合并库文件
    local universal_dir="$SCRIPT_DIR/build_macos_universal_${build_type}"
    mkdir -p "$universal_dir"
    cd "$universal_dir"
    
    # 创建通用库目录结构
    mkdir -p "$OUTPUT_DIR/macOS/universal/$build_type"
    
    # 合并静态库
    if [ "$shared" = "OFF" ]; then
        log_info "合并macOS静态库为通用库"
        lipo -create \
            "$PROJECT_ROOT/Bin/macOS/x86_64/$build_type/libCYLogger.a" \
            "$PROJECT_ROOT/Bin/macOS/arm64/$build_type/libCYLogger.a" \
            -output "$OUTPUT_DIR/macOS/universal/$build_type/libCYLogger.a"
    else
        # 合并动态库
        log_info "合并macOS动态库为通用库"
        lipo -create \
            "$PROJECT_ROOT/Bin/macOS/x86_64/$build_type/libCYLogger.dylib" \
            "$PROJECT_ROOT/Bin/macOS/arm64/$build_type/libCYLogger.dylib" \
            -output "$OUTPUT_DIR/macOS/universal/$build_type/libCYLogger.dylib"
    fi
    
    log_info "完成macOS通用库构建"
}

# iOS通用库构建函数
build_ios_universal() {
    local build_type=$1
    local shared=$2
    local deployment_target=$3
    
    log_info "构建iOS通用库 $build_type ${shared:+动态库} ${shared:-静态库}"
    
    # 分别构建不同架构
    local archs=("arm64" "x86_64")
    local build_dirs=()
    
    for arch in "${archs[@]}"; do
        local arch_build_dir="$SCRIPT_DIR/build_ios_${arch}_${build_type}"
        mkdir -p "$arch_build_dir"
        cd "$arch_build_dir"
        
        log_info "构建iOS $arch $build_type ${shared:+动态库} ${shared:-静态库}"
        
        cmake "$PROJECT_ROOT" \
            "-DCMAKE_BUILD_TYPE=$build_type" \
            "-DCMAKE_CXX_STANDARD=20" \
            "-DBUILD_SHARED_LIBS=$shared" \
            "-DCMAKE_SYSTEM_NAME=iOS" \
            "-DCMAKE_OSX_DEPLOYMENT_TARGET=$deployment_target" \
            "-DCMAKE_OSX_ARCHITECTURES=$arch"
        
        make -j$(sysctl -n hw.ncpu)
        build_dirs+=("$arch_build_dir")
    done
    
    # 合并库文件
    local universal_dir="$SCRIPT_DIR/build_ios_universal_${build_type}"
    mkdir -p "$universal_dir"
    cd "$universal_dir"
    
    # 创建通用库目录结构
    mkdir -p "$OUTPUT_DIR/iOS/universal/$build_type"
    
    # 合并静态库
    if [ "$shared" = "OFF" ]; then
        log_info "合并iOS静态库为通用库"
        lipo -create \
            "$PROJECT_ROOT/Bin/iOS/arm64/$build_type/libCYLogger.a" \
            "$PROJECT_ROOT/Bin/iOS/x86_64/$build_type/libCYLogger.a" \
            -output "$OUTPUT_DIR/iOS/universal/$build_type/libCYLogger.a"
    else
        # 合并动态库
        log_info "合并iOS动态库为通用库"
        lipo -create \
            "$PROJECT_ROOT/Bin/arm64/$build_type/libCYLogger.dylib" \
            "$PROJECT_ROOT/Bin/x86_64/$build_type/libCYLogger.dylib" \
            -output "$OUTPUT_DIR/ios_universal/$build_type/libCYLogger.dylib"
    fi
    
    log_info "完成iOS通用库构建"
}

# 主函数
main() {
    log_info "开始跨平台构建CYLogger库"
    
    # macOS构建
    log_info "=== 开始构建macOS平台 ==="
    build_platform "macos" "universal" "Release" "OFF" "11.0"  # 静态库
    build_platform "macos" "universal" "Release" "ON" "11.0"   # 动态库
    build_platform "macos" "universal" "Debug" "OFF" "11.0"    # 静态库
    build_platform "macos" "universal" "Debug" "ON" "11.0"     # 动态库
    
    # iOS构建
    log_info "=== 开始构建iOS平台 ==="
    build_platform "ios" "universal" "Release" "OFF" "11.0"  # 静态库
    build_platform "ios" "universal" "Release" "ON" "11.0"   # 动态库
    build_platform "ios" "universal" "Debug" "OFF" "11.0"    # 静态库
    build_platform "ios" "universal" "Debug" "ON" "11.0"     # 动态库
    
    # Linux构建 (如果在Linux系统上)
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        log_info "=== 开始构建Linux平台 ==="
        build_platform "linux" "x64" "Release" "OFF" ""  # 静态库
        build_platform "linux" "x64" "Release" "ON" ""   # 动态库
        build_platform "linux" "x64" "Debug" "OFF" ""    # 静态库
        build_platform "linux" "x64" "Debug" "ON" ""     # 动态库
    fi
    
    # Windows构建 (如果在Windows系统上)
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        log_info "=== 开始构建Windows平台 ==="
        build_platform "windows" "x64" "Release" "OFF" ""  # 静态库
        build_platform "windows" "x64" "Release" "ON" ""   # 动态库
        build_platform "windows" "x64" "Debug" "OFF" ""    # 静态库
        build_platform "windows" "x64" "Debug" "ON" ""     # 动态库
    fi
    
    log_info "所有平台构建完成！"
    log_info "输出目录: $OUTPUT_DIR"
}

# 执行主函数
main "$@"