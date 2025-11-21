@echo off
echo Building CYLogger for Windows...

REM 设置变量
set BUILD_DIR=%~dp0
set PROJECT_ROOT=%BUILD_DIR%..
set BUILD_TYPE=%1
set LIB_TYPE=%2

REM 默认参数
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%LIB_TYPE%"=="" set LIB_TYPE=Static

REM 创建构建目录
if "%LIB_TYPE%"=="Static" (
    set BUILD_SUBDIR=build_windows_static_%BUILD_TYPE%
) else (
    set BUILD_SUBDIR=build_windows_shared_%BUILD_TYPE%
)

set BUILD_PATH=%BUILD_DIR%%BUILD_SUBDIR%
if not exist "%BUILD_PATH%" mkdir "%BUILD_PATH%"

cd /d "%BUILD_PATH%"

REM 配置CMake
echo Configuring CMake...
if "%LIB_TYPE%"=="Static" (
    cmake -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DBUILD_SHARED_LIBS=OFF ^
        -DBUILD_EXAMPLES=ON ^
        "%PROJECT_ROOT%"
) else (
    cmake -G "Visual Studio 17 2022" -A x64 ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DBUILD_SHARED_LIBS=ON ^
        -DBUILD_EXAMPLES=ON ^
        "%PROJECT_ROOT%"
)

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM 编译
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Output directory: %PROJECT_ROOT%\Bin\Windows\x64\%BUILD_TYPE%

cd /d "%BUILD_DIR%"
echo.
echo To run the console test:
echo %PROJECT_ROOT%\Bin\Windows\x64\%BUILD_TYPE%\CYLoggerConsoleTest.exe
echo.