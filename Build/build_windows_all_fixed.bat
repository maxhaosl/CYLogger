@echo off
setlocal enableextensions enabledelayedexpansion

set BUILD_DIR=%~dp0
set PROJECT_ROOT=%BUILD_DIR%..

set BUILD_PARALLEL=%NUMBER_OF_PROCESSORS%
if "%BUILD_PARALLEL%"=="" set BUILD_PARALLEL=4

set ARCH_LIST=%1
if "%ARCH_LIST%"=="" set ARCH_LIST=x64
set LIB_LIST=%2
if "%LIB_LIST%"=="" set LIB_LIST=Static,Shared
set BUILD_LIST=%3
if "%BUILD_LIST%"=="" set BUILD_LIST=Release,Debug
set CRT_LIST=%4
if "%CRT_LIST%"=="" set CRT_LIST=MD,MT

set ARCH_LIST=%ARCH_LIST:,= %
set LIB_LIST=%LIB_LIST:,= %
set BUILD_LIST=%BUILD_LIST:,= %
set CRT_LIST=%CRT_LIST:,= %

echo Building CYLogger for Windows matrix...
echo Architectures: %ARCH_LIST%
echo Library kinds: %LIB_LIST%
echo Build types : %BUILD_LIST%
echo CRT runtimes: %CRT_LIST%
echo.

for %%A in (%ARCH_LIST%) do (
    for %%L in (%LIB_LIST%) do (
        for %%C in (%BUILD_LIST%) do (
            for %%R in (%CRT_LIST%) do (
                call :ValidateBuildRuntimeCombination "%%C" "%%R"
                if not errorlevel 1 (
                    echo ========================================
                    echo Arch: %%A ^| Lib: %%L ^| Build: %%C ^| CRT: %%R
                    echo ========================================
                    call :EnsureDependencies %%A %%C %%L %%R
                    if errorlevel 1 (
                        echo Failed to prepare dependencies for %%A / %%L / %%C / %%R
                        exit /b 1
                    )
                    call "%BUILD_DIR%build_windows.bat" %%C %%L %%A %%R
                    if errorlevel 1 (
                        echo Failed for %%A / %%L / %%C / %%R
                        exit /b 1
                    )
                ) else (
                    echo Skipping invalid combination: %%C + %%R
                )
            )
        )
    )
)

echo.
echo ========================================
echo All Windows builds completed successfully
echo ========================================

echo.
echo Generated libraries:
powershell -NoProfile -Command "Get-ChildItem -Path '%PROJECT_ROOT%\Bin\Windows' -Include *.lib,*.dll -Recurse -ErrorAction SilentlyContinue | Sort-Object FullName | ForEach-Object { $_.FullName }"

endlocal

:EnsureDependencies
setlocal enabledelayedexpansion
set ARCH=%~1
set CONFIG=%~2
set KIND=%~3
set RUNTIME_INPUT=%~4

if "%RUNTIME_INPUT%"=="" set RUNTIME_INPUT=MD
set RUNTIME_INPUT=%RUNTIME_INPUT:"=%

call :NormalizeBuildTypeVar CONFIG
if errorlevel 1 (
    endlocal & exit /b 1
)

call :ValidateRuntimeVar RUNTIME_INPUT
if errorlevel 1 (
    endlocal & exit /b 1
)

set BASE_RUNTIME=!RUNTIME_INPUT!
call :ResolveRuntimeForConfig "!CONFIG!" "!BASE_RUNTIME!" RESOLVED_RUNTIME
if errorlevel 1 (
    endlocal & exit /b 1
)
set RUNTIME_FOR_CONFIG=!RESOLVED_RUNTIME!

set ARCH_DIR=%PROJECT_ROOT%\Bin\Windows\%ARCH%\!RUNTIME_FOR_CONFIG!\%CONFIG%
set NEED_STATIC=0
if not exist "%ARCH_DIR%\CYCoroutine.lib" set NEED_STATIC=1

set NEED_SHARED=0
if /I "%KIND%"=="Shared" (
    if not exist "%ARCH_DIR%\CYCoroutine.dll" set NEED_SHARED=1
)

if %NEED_STATIC%==0 if %NEED_SHARED%==0 (
    echo CYCoroutine dependencies already present for %ARCH% / %CONFIG% / %KIND% / !RUNTIME_FOR_CONFIG!
    endlocal & exit /b 0
)

set SHARED_FLAG=OFF
if /I "%KIND%"=="Shared" set SHARED_FLAG=ON

set DEP_DIR=%BUILD_DIR%deps_windows_%ARCH%_!RUNTIME_FOR_CONFIG!_%CONFIG%_%SHARED_FLAG%
echo Preparing CYCoroutine dependencies in %DEP_DIR%...
cmake -S "%PROJECT_ROOT%" -B "%DEP_DIR%" -G "Visual Studio 17 2022" -A %ARCH% -DCMAKE_BUILD_TYPE=%CONFIG% -DBUILD_SHARED_LIBS=%SHARED_FLAG% -DBUILD_EXAMPLES=OFF -DCYLOGGER_MSVC_RUNTIME=!RUNTIME_FOR_CONFIG!
if errorlevel 1 (
    endlocal & exit /b 1
)

if %NEED_STATIC%==1 (
    echo Building CYCoroutine_static for %ARCH% / %CONFIG% / !RUNTIME_FOR_CONFIG!...
    cmake --build "%DEP_DIR%" --config %CONFIG% --target CYCoroutine_static --parallel %BUILD_PARALLEL%
    if errorlevel 1 (
        endlocal & exit /b 1
    )
    REM Copy the static library to the target directory
    if not exist "%ARCH_DIR%" mkdir "%ARCH_DIR%"
    for %%F in ("%DEP_DIR%\Bin\Windows\%ARCH%\!RUNTIME_FOR_CONFIG!\%CONFIG%\CYCoroutine.lib") do (
        if exist "%%F" (
            copy /Y "%%F" "%ARCH_DIR%\CYCoroutine.lib" >nul
            echo Copied CYCoroutine.lib to %ARCH_DIR%
        ) else (
            echo Warning: CYCoroutine.lib not found at expected location: %%F
        )
    )
)

if %NEED_SHARED%==1 (
    echo Building CYCoroutine_shared for %ARCH% / %CONFIG% / !RUNTIME_FOR_CONFIG!...
    cmake --build "%DEP_DIR%" --config %CONFIG% --target CYCoroutine_shared --parallel %BUILD_PARALLEL%
    if errorlevel 1 (
        endlocal & exit /b 1
    )
    REM Copy the shared library to the target directory
    if not exist "%ARCH_DIR%" mkdir "%ARCH_DIR%"
    for %%F in ("%DEP_DIR%\Bin\Windows\%ARCH%\!RUNTIME_FOR_CONFIG!\%CONFIG%\CYCoroutine.dll") do (
        if exist "%%F" (
            copy /Y "%%F" "%ARCH_DIR%\CYCoroutine.dll" >nul
            echo Copied CYCoroutine.dll to %ARCH_DIR%
        ) else (
            echo Warning: CYCoroutine.dll not found at expected location: %%F
        )
    )
)

endlocal & exit /b 0

:ValidateRuntimeVar
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

:NormalizeBuildTypeVar
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

:ResolveRuntimeForConfig
set "CFG=%~1"
set "BASE=%~2"
set "OUT_VAR=%~3"

if /I "%CFG%"=="Debug" (
    REM For Debug, accept MDD, MTD directly, or resolve MD/MT to MDD/MTD
    if /I "%BASE%"=="MDD" (
        set RESULT=MDD
    ) else if /I "%BASE%"=="MTD" (
        set RESULT=MTD
    ) else if /I "%BASE%"=="MT" (
        set RESULT=MTD
    ) else (
        REM Default to MDD for Debug if MD or unspecified
        set RESULT=MDD
    )
) else if /I "%CFG%"=="Release" (
    REM For Release, only accept MD or MT (not MDD or MTD)
    if /I "%BASE%"=="MT" (
        set RESULT=MT
    ) else if /I "%BASE%"=="MDD" (
        echo Error: MDD is not valid for Release build. Use MD instead.
        exit /b 1
    ) else if /I "%BASE%"=="MTD" (
        echo Error: MTD is not valid for Release build. Use MT instead.
        exit /b 1
    ) else (
        REM Default to MD for Release
        set RESULT=MD
    )
) else (
    echo Unsupported build type "%CFG%".
    exit /b 1
)

set "%OUT_VAR%=%RESULT%"
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