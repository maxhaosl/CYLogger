@echo off

REM Configure paths
set BUILD_DIR=%~dp0
set PROJECT_ROOT=%BUILD_DIR%..
set BUILD_TYPE=%1
set LIB_TYPE=%2
set TARGET_ARCH=%3
set CRT_TYPE=%4

REM Default arguments
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%LIB_TYPE%"=="" set LIB_TYPE=Static
if "%TARGET_ARCH%"=="" set TARGET_ARCH=x86_64
if "%CRT_TYPE%"=="" set CRT_TYPE=MD

set BUILD_TYPE=%BUILD_TYPE:"=%
set LIB_TYPE=%LIB_TYPE:"=%
set TARGET_ARCH=%TARGET_ARCH:"=%
set CRT_TYPE=%CRT_TYPE:"=%

REM Map architecture: x64 -> x86_64 for output paths, but keep x64 for CMake -A parameter
set OUTPUT_ARCH=%TARGET_ARCH%
if /I "%TARGET_ARCH%"=="x64" set OUTPUT_ARCH=x86_64
REM For CMake -A parameter, Visual Studio uses x64, not x86_64
set CMAKE_ARCH=%TARGET_ARCH%
if /I "%TARGET_ARCH%"=="x86_64" set CMAKE_ARCH=x64

call :NormalizeBuildType BUILD_TYPE
if errorlevel 1 exit /b 1

call :ValidateRuntime CRT_TYPE
if errorlevel 1 exit /b 1

set EFFECTIVE_RUNTIME=
call :ResolveRuntimeForConfig "%BUILD_TYPE%" "%CRT_TYPE%"
if errorlevel 1 exit /b 1

echo Building CYLogger for Windows (%OUTPUT_ARCH%) with %EFFECTIVE_RUNTIME% runtime...

REM Create build directory (use OUTPUT_ARCH for directory names)
if /I "%LIB_TYPE%"=="Static" (
    set BUILD_SUBDIR=build_windows_%OUTPUT_ARCH%_%EFFECTIVE_RUNTIME%_static_%BUILD_TYPE%
) else (
    set BUILD_SUBDIR=build_windows_%OUTPUT_ARCH%_%EFFECTIVE_RUNTIME%_shared_%BUILD_TYPE%
)

set BUILD_PATH=%BUILD_DIR%%BUILD_SUBDIR%
if not exist "%BUILD_PATH%" mkdir "%BUILD_PATH%"

cd /d "%BUILD_PATH%"

REM Configure CMake
echo Configuring CMake...
REM Use CMAKE_ARCH for -A parameter (Visual Studio uses x64, not x86_64)
REM Use OUTPUT_ARCH for TARGET_ARCH to ensure output paths use x86_64
if /I "%LIB_TYPE%"=="Static" (
    cmake -G "Visual Studio 17 2022" -A %CMAKE_ARCH% ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DBUILD_SHARED_LIBS=OFF ^
        -DBUILD_EXAMPLES=ON ^
        -DCYLOGGER_MSVC_RUNTIME=%EFFECTIVE_RUNTIME% ^
        -DTARGET_ARCH=%OUTPUT_ARCH% ^
        "%PROJECT_ROOT%"
) else (
    cmake -G "Visual Studio 17 2022" -A %CMAKE_ARCH% ^
        -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
        -DBUILD_SHARED_LIBS=ON ^
        -DBUILD_EXAMPLES=ON ^
        -DCYLOGGER_MSVC_RUNTIME=%EFFECTIVE_RUNTIME% ^
        -DTARGET_ARCH=%OUTPUT_ARCH% ^
        "%PROJECT_ROOT%"
)

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo Output directory: %PROJECT_ROOT%\Bin\Windows\%OUTPUT_ARCH%\%EFFECTIVE_RUNTIME%\%BUILD_TYPE%

cd /d "%BUILD_DIR%"
echo.
echo To run the console test:
echo %PROJECT_ROOT%\Bin\Windows\%OUTPUT_ARCH%\%EFFECTIVE_RUNTIME%\%BUILD_TYPE%\CYLoggerConsoleTest.exe
echo.
goto :eof

:NormalizeBuildType
set "VAR_NAME=%~1"
call set "CURRENT_VALUE=%%%VAR_NAME%%%"
if "%CURRENT_VALUE%"=="" set "CURRENT_VALUE=Release"

if /I "%CURRENT_VALUE%"=="DEBUG" (
    set CURRENT_VALUE=Debug
) else if /I "%CURRENT_VALUE%"=="RELEASE" (
    set CURRENT_VALUE=Release
) else (
    echo Invalid build type "%CURRENT_VALUE%". Use Release or Debug.
    exit /b 1
)

set "%VAR_NAME%=%CURRENT_VALUE%"
exit /b 0

:ValidateRuntime
set "VAR_NAME=%~1"
call set "CURRENT_VALUE=%%%VAR_NAME%%%"
if "%CURRENT_VALUE%"=="" set "CURRENT_VALUE=MD"
set "CURRENT_VALUE=%CURRENT_VALUE:"=%"

REM Convert to uppercase for comparison
set "CURRENT_VALUE_UPPER=%CURRENT_VALUE%"
if /I "%CURRENT_VALUE_UPPER%"=="MT" (
    set CURRENT_VALUE=MT
) else if /I "%CURRENT_VALUE_UPPER%"=="MD" (
    set CURRENT_VALUE=MD
) else if /I "%CURRENT_VALUE_UPPER%"=="MTD" (
    set CURRENT_VALUE=MTD
) else if /I "%CURRENT_VALUE_UPPER%"=="MDD" (
    set CURRENT_VALUE=MDD
) else (
    echo Invalid CRT runtime "%CURRENT_VALUE%". Use MT, MD, MTD, or MDD.
    exit /b 1
)

set "%VAR_NAME%=%CURRENT_VALUE%"
exit /b 0

:ResolveRuntimeForConfig
set "CFG=%~1"
set "BASE=%~2"

if /I "%CFG%"=="Debug" (
    REM For Debug, accept MDD, MTD directly, or resolve MD/MT to MDD/MTD
    if /I "%BASE%"=="MDD" (
        set EFFECTIVE_RUNTIME=MDD
    ) else if /I "%BASE%"=="MTD" (
        set EFFECTIVE_RUNTIME=MTD
    ) else if /I "%BASE%"=="MT" (
        set EFFECTIVE_RUNTIME=MTD
    ) else (
        REM Default to MDD for Debug if MD or unspecified
        set EFFECTIVE_RUNTIME=MDD
    )
) else if /I "%CFG%"=="Release" (
    REM For Release, only accept MD or MT (not MDD or MTD)
    if /I "%BASE%"=="MT" (
        set EFFECTIVE_RUNTIME=MT
    ) else if /I "%BASE%"=="MDD" (
        echo Error: MDD is not valid for Release build. Use MD instead.
        exit /b 1
    ) else if /I "%BASE%"=="MTD" (
        echo Error: MTD is not valid for Release build. Use MT instead.
        exit /b 1
    ) else (
        REM Default to MD for Release
        set EFFECTIVE_RUNTIME=MD
    )
) else (
    echo Unsupported build type "%CFG%".
    exit /b 1
)

exit /b 0