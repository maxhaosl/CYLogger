@echo off
setlocal enableextensions enabledelayedexpansion

set "BUILD_DIR=%~dp0"
set "PROJECT_ROOT=%BUILD_DIR%.."
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"

set BUILD_PARALLEL=%NUMBER_OF_PROCESSORS%
if "%BUILD_PARALLEL%"=="" set BUILD_PARALLEL=4

set ARCH_LIST=%1
if "%ARCH_LIST%"=="" set ARCH_LIST=x86_64,x86
set BUILD_LIST=%2
if "%BUILD_LIST%"=="" set BUILD_LIST=Release,Debug
set CRT_LIST=%3
if "%CRT_LIST%"=="" set CRT_LIST=MD,MT

call :EnsureFmtSubmodule
if errorlevel 1 exit /b 1

call :EnsureCYCommonSubmodule
if errorlevel 1 exit /b 1

set ARCH_LIST=%ARCH_LIST:,= %
set BUILD_LIST=%BUILD_LIST:,= %
set CRT_LIST=%CRT_LIST:,= %

echo Building CYLogger for Windows matrix...
echo Architectures: %ARCH_LIST%
echo Build types : %BUILD_LIST%
echo CRT runtimes: %CRT_LIST%
echo.

for %%A in (%ARCH_LIST%) do (
    for %%C in (%BUILD_LIST%) do (
        for %%R in (%CRT_LIST%) do (
            call :ValidateBuildRuntimeCombination "%%C" "%%R"
            if not errorlevel 1 (
                call :ResolveRuntimeForMain "%%C" "%%R" RESOLVED_CRT
                if errorlevel 1 exit /b 1
                echo ========================================
                echo Arch: %%A ^| Build: %%C ^| CRT: !RESOLVED_CRT!
                echo ========================================
                call :EnsureDependencies %%A %%C !RESOLVED_CRT!
                if errorlevel 1 (
                    echo Failed to prepare dependencies for %%A / %%C / !RESOLVED_CRT!
                    exit /b 1
                )
                call "%BUILD_DIR%build_windows.bat" %%C %%A !RESOLVED_CRT!
                if errorlevel 1 (
                    echo Failed for %%A / %%C / !RESOLVED_CRT!
                    exit /b 1
                )
            ) else (
                echo Skipping invalid combination: %%C + %%R
            )
        )
    )
)

REM Clean up temporary build directories (do NOT remove CYCommon/CYCoroutine Bin directories)
if exist "%BUILD_DIR%deps_*" (
    rmdir /s /q "%BUILD_DIR%deps_*" 2>nul
)

echo.
echo ========================================
echo All Windows builds completed successfully
echo ========================================

echo.
echo Generated libraries:
powershell -NoProfile -Command "Get-ChildItem -Path '%PROJECT_ROOT%\Bin\Windows' -Include *.lib,*.dll -Recurse -ErrorAction SilentlyContinue | Sort-Object FullName | ForEach-Object { $_.FullName }"

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

:EnsureDependencies
set "ARCH=%~1"
set "CONFIG=%~2"
set "RUNTIME_INPUT=%~3"

if "%RUNTIME_INPUT%"=="" set "RUNTIME_INPUT=MD"
set "RUNTIME_INPUT=%RUNTIME_INPUT:"=%"

REM Normalize build type
set "CONFIG=%CONFIG%"
if /I "%CONFIG%"=="DEBUG" set "CONFIG=Debug"
if /I "%CONFIG%"=="RELEASE" set "CONFIG=Release"

REM Normalize runtime
set "RUNTIME=%RUNTIME_INPUT%"
if /I "%RUNTIME%"=="MT" set "RUNTIME=MT"
if /I "%RUNTIME%"=="MD" set "RUNTIME=MD"
if /I "%RUNTIME%"=="MTD" set "RUNTIME=MTD"
if /I "%RUNTIME%"=="MDD" set "RUNTIME=MDD"

REM Map architecture: x64 -> x86_64 for output paths
REM x86 stays as x86 for output paths
REM For CMake -A parameter: x86_64 uses x64, x86 uses Win32
set "OUTPUT_ARCH=%ARCH%"
if /I "%ARCH%"=="x64" set "OUTPUT_ARCH=x86_64"
set "CMAKE_ARCH=%ARCH%"
if /I "%ARCH%"=="x86_64" set "CMAKE_ARCH=x64"
if /I "%ARCH%"=="x86" set "CMAKE_ARCH=Win32"

REM Resolve runtime for build type
set "RESOLVED_RUNTIME=%RUNTIME%"
if /I "%CONFIG%"=="Debug" (
    if /I "%RUNTIME%"=="MT" set "RESOLVED_RUNTIME=MTD"
    if /I "%RUNTIME%"=="MD" set "RESOLVED_RUNTIME=MDD"
)
if /I "%CONFIG%"=="Release" (
    if /I "%RUNTIME%"=="MTD" (
        echo Error: MTD is not valid for Release build. Use MT instead.
        exit /b 1
    )
    if /I "%RUNTIME%"=="MDD" (
        echo Error: MDD is not valid for Release build. Use MD instead.
        exit /b 1
    )
)

REM Set up directory paths and library names before check_deps
set "ARCH_DIR=%PROJECT_ROOT%\Bin\Windows\%OUTPUT_ARCH%\%RUNTIME%\%CONFIG%"
if /I "%CONFIG%"=="Debug" (
    set "CYCOMMON_LIB_NAME=CYCommonD.lib"
) else (
    set "CYCOMMON_LIB_NAME=CYCommon.lib"
)

:check_deps
REM Always build CYCommon for this specific arch/config/runtime combination.
REM Different CRT types (MD vs MT) produce different binaries; we cannot skip.
REM
REM Note: ARCH_DIR and CYCOMMON_LIB_NAME are set up in :EnsureDependencies
REM before this label is called.
REM
REM This label is kept for reference but the skip logic below is intentionally disabled.
REM If you want to re-enable skip logic, add back the NEED_BUILD_DEPS check
REM but it MUST account for CRT type (MD vs MT) to avoid cross-contamination.
REM CURRENTLY DISABLED:
set "NEED_BUILD_DEPS=0"

REM ==============================================================================
REM Step 1: Build CYCommon first (in CYCommon's own Bin directory)
REM ==============================================================================

echo.
echo ========================================
echo Building CYCommon for %OUTPUT_ARCH% / %CONFIG% / %RUNTIME%
echo ========================================
set "CYCOMMON_BUILD_SCRIPT=%PROJECT_ROOT%\ThirdParty\CYCommon\Build\build_windows.bat"
call "!CYCOMMON_BUILD_SCRIPT!" %CONFIG% Static %OUTPUT_ARCH% %RUNTIME%
if errorlevel 1 (
    echo CYCommon build failed
    exit /b 1
)
echo CYCommon built successfully

REM ==============================================================================
REM Step 2: Copy CYCommon library to CYLogger's Bin directory
REM ==============================================================================

set "CYCOMMON_SRC=%PROJECT_ROOT%\ThirdParty\CYCommon\Bin\Windows\%OUTPUT_ARCH%\%RUNTIME%\%CONFIG%"
if not exist "%ARCH_DIR%" mkdir "%ARCH_DIR%" 2>nul
if exist "%CYCOMMON_SRC%\%CYCOMMON_LIB_NAME%" (
    copy /Y "%CYCOMMON_SRC%\%CYCOMMON_LIB_NAME%" "%ARCH_DIR%\" >nul 2>&1
    REM Copy PDB
    if /I "%CONFIG%"=="Debug" (
        copy /Y "%CYCOMMON_SRC%\CYCommonD.pdb" "%ARCH_DIR%\" >nul 2>&1
    ) else (
        copy /Y "%CYCOMMON_SRC%\CYCommon.pdb" "%ARCH_DIR%\" >nul 2>&1
    )
    echo CYCommon copied to CYLogger Bin
)

REM ==============================================================================
REM Step 3: Build CYCoroutine
REM CYCoroutine will find the CYCommon we just built
REM ==============================================================================

echo.
echo ========================================
echo Building CYCoroutine for %OUTPUT_ARCH% / %CONFIG% / %RUNTIME%
echo ========================================

REM Build CYCoroutine as static library
set "CYCOMMON_DEP_DIR=%BUILD_DIR%deps_windows_%OUTPUT_ARCH%_%RUNTIME%_%CONFIG%_static"
if exist "%CYCOMMON_DEP_DIR%" rmdir /s /q "%CYCOMMON_DEP_DIR%" 2>nul
if exist "%CYCOMMON_DEP_DIR%\CMakeCache.txt" del /f /q "%CYCOMMON_DEP_DIR%\CMakeCache.txt" >nul 2>&1
mkdir "%CYCOMMON_DEP_DIR%" 2>nul

cmake -S "%PROJECT_ROOT%" -B "%CYCOMMON_DEP_DIR%" -G "Visual Studio 17 2022" -A %CMAKE_ARCH% -DCMAKE_BUILD_TYPE=%CONFIG% -DBUILD_SHARED_LIBS=OFF -DBUILD_EXAMPLES=OFF -DCYLOGGER_MSVC_RUNTIME=%RUNTIME% -DWINDOWS_RUNTIME=%RUNTIME% -DTARGET_ARCH=%OUTPUT_ARCH%
if errorlevel 1 (
    echo CMake configuration failed for CYCoroutine
    exit /b 1
)

cmake --build "%CYCOMMON_DEP_DIR%" --config %CONFIG% --target CYCoroutine_static --parallel %BUILD_PARALLEL%
if errorlevel 1 (
    echo CYCoroutine build failed
    exit /b 1
)

REM Copy CYCoroutine to CYLogger's Bin directory
set "CYCOROUTINE_SRC=%CYCOMMON_DEP_DIR%\ThirdParty\CYCoroutine\Bin\Windows\%OUTPUT_ARCH%\%RUNTIME%\%CONFIG%"
if /I "%CONFIG%"=="Debug" (
    set "CYCOROUTINE_LIB_NAME=CYCoroutineD.lib"
) else (
    set "CYCOROUTINE_LIB_NAME=CYCoroutine.lib"
)
if exist "%CYCOROUTINE_SRC%\%CYCOROUTINE_LIB_NAME%" (
    copy /Y "%CYCOROUTINE_SRC%\%CYCOROUTINE_LIB_NAME%" "%ARCH_DIR%\" >nul 2>&1
    if /I "%CONFIG%"=="Debug" (
        copy /Y "%CYCOROUTINE_SRC%\CYCoroutineD.pdb" "%ARCH_DIR%\" >nul 2>&1
    ) else (
        copy /Y "%CYCOROUTINE_SRC%\CYCoroutine.pdb" "%ARCH_DIR%\" >nul 2>&1
    )
    echo CYCoroutine copied to CYLogger Bin
)

exit /b 0

:ValidateBuildRuntimeCombination
set "CFG=%~1"
set "RUNTIME=%~2"
set "RUNTIME=%RUNTIME:"=%"

REM Only allow these four valid combinations (after resolution):
REM Debug + MDD, Debug + MTD, Release + MT, Release + MD
REM For Debug: accept MDD, MTD, MD (will be resolved to MDD), MT (will be resolved to MTD)
REM For Release: accept MD, MT only (MDD/MTD are not valid for Release)
if /I "%CFG%"=="Debug" (
    if /I "%RUNTIME%"=="MDD" exit /b 0
    if /I "%RUNTIME%"=="MTD" exit /b 0
    if /I "%RUNTIME%"=="MD" exit /b 0
    if /I "%RUNTIME%"=="MT" exit /b 0
    exit /b 1
) else if /I "%CFG%"=="Release" (
    REM Release only accepts MD or MT, not MDD or MTD
    if /I "%RUNTIME%"=="MD" exit /b 0
    if /I "%RUNTIME%"=="MT" exit /b 0
    exit /b 1
) else (
    exit /b 1
)

exit /b 0

:ResolveRuntimeForMain
set "CFG=%~1"
set "RUNTIME=%~2"
set "OUT_VAR=%~3"
if /I "%CFG%"=="Debug" (
    if /I "%RUNTIME%"=="MD" (
        set "%OUT_VAR%=MDD"
    ) else if /I "%RUNTIME%"=="MT" (
        set "%OUT_VAR%=MTD"
    ) else (
        set "%OUT_VAR%=%RUNTIME%"
    )
) else (
    set "%OUT_VAR%=%RUNTIME%"
)
exit /b 0
