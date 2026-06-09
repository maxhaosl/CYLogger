@echo off

REM Configure paths
set BUILD_DIR=%~dp0
set PROJECT_ROOT=%BUILD_DIR%..
for %%I in ("%PROJECT_ROOT%") do set PROJECT_ROOT=%%~fI
set BUILD_TYPE=%1
set TARGET_ARCH=%2
set CRT_TYPE=%3

REM Default arguments
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%TARGET_ARCH%"=="" set TARGET_ARCH=x86_64
if "%CRT_TYPE%"=="" set CRT_TYPE=MD

set BUILD_TYPE=%BUILD_TYPE:"=%
set TARGET_ARCH=%TARGET_ARCH:"=%
set CRT_TYPE=%CRT_TYPE:"=%

call :EnsureFmtSubmodule
if errorlevel 1 exit /b 1

call :EnsureCYCommonSubmodule
if errorlevel 1 exit /b 1

REM Map architecture: x64 -> x86_64 for output paths
REM x86 stays as x86 for output paths
REM For CMake -A parameter: x86_64 uses x64, x86 uses Win32
set OUTPUT_ARCH=%TARGET_ARCH%
if /I "%TARGET_ARCH%"=="x64" set OUTPUT_ARCH=x86_64
set CMAKE_ARCH=%TARGET_ARCH%
if /I "%TARGET_ARCH%"=="x86_64" set CMAKE_ARCH=x64
if /I "%TARGET_ARCH%"=="x86" set CMAKE_ARCH=Win32

call :NormalizeBuildType BUILD_TYPE
if errorlevel 1 exit /b 1

call :ValidateRuntime CRT_TYPE
if errorlevel 1 exit /b 1

set EFFECTIVE_RUNTIME=
call :ResolveRuntimeForConfig "%BUILD_TYPE%" "%CRT_TYPE%"
if errorlevel 1 exit /b 1

REM Pass resolved values as arguments to EnsureCYCommon (subroutine creates a new scope)
call :EnsureCYCommon "%TARGET_ARCH%" "%BUILD_TYPE%" "%EFFECTIVE_RUNTIME%" "%OUTPUT_ARCH%" "%CMAKE_ARCH%"
if errorlevel 1 exit /b 1

echo Building CYLogger for Windows (%OUTPUT_ARCH%) with %EFFECTIVE_RUNTIME% runtime...
echo Building both Static and Shared libraries in a single CMake run.

REM Create build directory (shared name, single directory for both static and shared)
set BUILD_SUBDIR=build_windows_%OUTPUT_ARCH%_%EFFECTIVE_RUNTIME%_%BUILD_TYPE%
set BUILD_PATH=%BUILD_DIR%%BUILD_SUBDIR%

REM Clean CMake cache to avoid cross-config pollution (e.g. wrong CYCommon::static IMPORTED path)
if exist "%BUILD_PATH%\CMakeCache.txt" del /f /q "%BUILD_PATH%\CMakeCache.txt" >nul 2>&1
if not exist "%BUILD_PATH%" mkdir "%BUILD_PATH%"

cd /d "%BUILD_PATH%"

REM Configure CMake - build both static AND shared libraries
echo Configuring CMake...
cmake -G "Visual Studio 17 2022" -A %CMAKE_ARCH% ^
    -UCMAKE_CONFIGURATION_TYPES ^
    -UBUILD_SHARED_LIBS ^
    -UWINDOWS_RUNTIME ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_SHARED_LIBS=ON ^
    -DBUILD_STATIC_LIBS=ON ^
    -DBUILD_EXAMPLES=ON ^
    -DCYLOGGER_MSVC_RUNTIME=%EFFECTIVE_RUNTIME% ^
    -DWINDOWS_RUNTIME=%EFFECTIVE_RUNTIME% ^
    -DTARGET_ARCH=%OUTPUT_ARCH% ^
    "%PROJECT_ROOT%"

if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

echo Building CYLoggerStatic...
cmake --build . --config %BUILD_TYPE% --target CYLoggerStatic --parallel 1

if %ERRORLEVEL% neq 0 (
    echo CYLoggerStatic build failed!
    exit /b 1
)

echo Building CYLogger_shared...
cmake --build . --config %BUILD_TYPE% --target CYLogger_shared --parallel 1

if %ERRORLEVEL% neq 0 (
    echo CYLogger_shared build failed!
    exit /b 1
)

echo Build completed successfully!
echo Output directory: %PROJECT_ROOT%\Bin\Windows\%OUTPUT_ARCH%\%EFFECTIVE_RUNTIME%\%BUILD_TYPE%

cd /d "%BUILD_DIR%"
echo.
echo Static library: CYLoggerStatic[D].lib
echo Shared library: CYLogger[D].lib / CYLogger[D].dll
goto :eof

:EnsureFmtSubmodule
set "FMT_HEADER=%PROJECT_ROOT%\ThirdParty\fmt\include\fmt\format.h"
if exist "%FMT_HEADER%" (
    exit /b 0
)

echo fmt headers not found. Initializing ThirdParty\fmt submodule...
pushd "%PROJECT_ROOT%" >nul
git submodule update --init --recursive ThirdParty/fmt
set "GIT_ERROR=%ERRORLEVEL%"
popd >nul

if not "%GIT_ERROR%"=="0" (
    echo Failed to update fmt submodule. Please check your git setup.
    exit /b %GIT_ERROR%
)

if not exist "%FMT_HEADER%" (
    echo fmt headers still missing after submodule update.
    exit /b 1
)

echo fmt submodule ready.
exit /b 0

:EnsureCYCommonSubmodule
set "CYCOMMON_DIR=%PROJECT_ROOT%\ThirdParty\CYCommon"
set "CYCOMMON_HEADER=%PROJECT_ROOT%\ThirdParty\CYCommon\Inc\CYCommon\CYCommon.hpp"
set "CYCOMMON_BUILD=%PROJECT_ROOT%\ThirdParty\CYCommon\Build\CMakeLists.txt"
if exist "%CYCOMMON_HEADER%" if exist "%CYCOMMON_BUILD%" (
    exit /b 0
)

REM CYCommon is expected to be a local copy (not a git submodule)
REM Check if it exists at the sibling location
set "CYCOMMON_SIBLING=%~dp0..\..\..\CYCommon"
if exist "%CYCOMMON_SIBLING%\Inc\CYCommon\CYCommon.hpp" (
    if exist "%CYCOMMON_SIBLING%\Build\CMakeLists.txt" (
        echo Found CYCommon at sibling location: %CYCOMMON_SIBLING%
        REM Create a symlink or copy would be needed here
        exit /b 0
    )
)

echo CYCommon not found at expected locations:
echo   - %CYCOMMON_DIR%
echo   - %CYCOMMON_SIBLING%
echo Please ensure CYCommon is available as a local dependency.
exit /b 1

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

:EnsureCYCommon
REM Build CYCommon static library into CYLogger/Bin/Windows if not already present.
REM This allows standalone use of build_windows.bat without build_all_windows.bat.
setlocal enabledelayedexpansion
set "PROJECT_ROOT=%~dp0.."
set "BUILD_DIR=%~dp0"
set "BUILD_PARALLEL=%NUMBER_OF_PROCESSORS%"
if "!BUILD_PARALLEL!"=="" set BUILD_PARALLEL=4

REM Arguments: TARGET_ARCH BUILD_TYPE EFFECTIVE_RUNTIME OUTPUT_ARCH CMAKE_ARCH
set "TARGET_ARCH=%~1"
set "BUILD_TYPE=%~2"
set "EFFECTIVE_RUNTIME=%~3"
set "OUTPUT_ARCH=%~4"
set "CMAKE_ARCH=%~5"

set "ARCH_DIR=%PROJECT_ROOT%\Bin\Windows\!OUTPUT_ARCH!\!EFFECTIVE_RUNTIME!\!BUILD_TYPE!"
if /I "!BUILD_TYPE!"=="Debug" (
    set "CYCOMMON_LIB_NAME=CYCommonD.lib"
) else (
    set "CYCOMMON_LIB_NAME=CYCommon.lib"
)

REM Check if CYCommon is already in CYLogger's Bin directory
if exist "!ARCH_DIR!\!CYCOMMON_LIB_NAME!" (
    echo CYCommon already present in CYLogger Bin: !ARCH_DIR!\!CYCOMMON_LIB_NAME!
    endlocal & exit /b 0
)

REM Check if CYCommon is in its own Bin directory and copy it
set "CYCOMMON_OWN_BIN=%PROJECT_ROOT%\ThirdParty\CYCommon\Bin\Windows\!OUTPUT_ARCH!\!EFFECTIVE_RUNTIME!\!BUILD_TYPE!\!CYCOMMON_LIB_NAME!"
if exist "!CYCOMMON_OWN_BIN!" (
    if not exist "!ARCH_DIR!" mkdir "!ARCH_DIR!" 2>nul
    copy /Y "!CYCOMMON_OWN_BIN!" "!ARCH_DIR!\" >nul 2>&1
    if /I "!BUILD_TYPE!"=="Debug" (
        copy /Y "%PROJECT_ROOT%\ThirdParty\CYCommon\Bin\Windows\!OUTPUT_ARCH!\!EFFECTIVE_RUNTIME!\!BUILD_TYPE!\CYCommonD.pdb" "!ARCH_DIR!\" >nul 2>&1
    ) else (
        copy /Y "%PROJECT_ROOT%\ThirdParty\CYCommon\Bin\Windows\!OUTPUT_ARCH!\!EFFECTIVE_RUNTIME!\!BUILD_TYPE!\CYCommon.pdb" "!ARCH_DIR!\" >nul 2>&1
    )
    echo CYCommon copied from CYCommon own Bin to CYLogger Bin
    endlocal & exit /b 0
)

REM Build CYCommon if not found
set "CYCOMMON_DEP_DIR=!BUILD_DIR!deps_cycommon_!OUTPUT_ARCH!_!EFFECTIVE_RUNTIME!_!BUILD_TYPE!"
if exist "!CYCOMMON_DEP_DIR!" rmdir /s /q "!CYCOMMON_DEP_DIR!" 2>nul
if exist "!CYCOMMON_DEP_DIR!\CMakeCache.txt" del /f /q "!CYCOMMON_DEP_DIR!\CMakeCache.txt" >nul 2>&1
mkdir "!CYCOMMON_DEP_DIR!" 2>nul

echo.
echo Building CYCommon for !OUTPUT_ARCH! / !BUILD_TYPE! / !EFFECTIVE_RUNTIME!...

cmake -S "!PROJECT_ROOT!\ThirdParty\CYCommon\Build" ^
      -B "!CYCOMMON_DEP_DIR!" ^
      -G "Visual Studio 17 2022" ^
      -A !CMAKE_ARCH! ^
      -DCMAKE_BUILD_TYPE=!BUILD_TYPE! ^
      -DBUILD_SHARED_LIBS=OFF ^
      -DWINDOWS_RUNTIME=!EFFECTIVE_RUNTIME! ^
      -DCYCOMMON_OUTPUT_DIR="!PROJECT_ROOT!\Bin\Windows\!OUTPUT_ARCH!\!EFFECTIVE_RUNTIME!"

if errorlevel 1 (
    echo CMake configuration failed for CYCommon
    endlocal & exit /b 1
)

cmake --build "!CYCOMMON_DEP_DIR!" --config !BUILD_TYPE! --target CYCommon_static --parallel !BUILD_PARALLEL!
if errorlevel 1 (
    echo CYCommon build failed
    endlocal & exit /b 1
)

echo CYCommon built successfully.
endlocal & exit /b 0