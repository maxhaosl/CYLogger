@echo off
setlocal enableextensions enabledelayedexpansion

set BUILD_DIR=%~dp0
set PROJECT_ROOT=%BUILD_DIR%..

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

echo ARCH_LIST=%ARCH_LIST%
echo LIB_LIST=%LIB_LIST%
echo BUILD_LIST=%BUILD_LIST%
echo CRT_LIST=%CRT_LIST%

for %%A in (%ARCH_LIST%) do (
    for %%L in (%LIB_LIST%) do (
        for %%C in (%BUILD_LIST%) do (
            for %%R in (%CRT_LIST%) do (
                echo Processing: %%A %%L %%C %%R
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
                ) else (
                    echo Skipping invalid combination: %%C + %%R
                )
            )
        )
    )
)

goto :eof

:EnsureDependencies
setlocal enabledelayedexpansion
echo EnsureDependencies called with: %1 %2 %3 %4
set ARCH=%~1
set CONFIG=%~2
set KIND=%~3
set RUNTIME_INPUT=%~4

echo ARCH=%ARCH%
echo CONFIG=%CONFIG%
echo KIND=%KIND%
echo RUNTIME_INPUT=%RUNTIME_INPUT%

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

echo RUNTIME_FOR_CONFIG=!RUNTIME_FOR_CONFIG!

endlocal & exit /b 0

:NormalizeBuildTypeVar
echo NormalizeBuildTypeVar called with: %1
set "VAR_NAME=%~1"
call set "CURRENT_VALUE=%%%VAR_NAME%%%"
echo CURRENT_VALUE=%CURRENT_VALUE%
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