@echo off
setlocal enabledelayedexpansion

call :EnsureDependencies x64 Release Static MD
goto :eof

:EnsureDependencies
setlocal enabledelayedexpansion
set ARCH=%~1
set CONFIG=%~2
set KIND=%~3
set RUNTIME_INPUT=%~4

if "%RUNTIME_INPUT%"=="" set RUNTIME_INPUT=MD
set RUNTIME_INPUT=%RUNTIME_INPUT:"=%

echo ARCH=%ARCH%
echo CONFIG=%CONFIG%
echo KIND=%KIND%
echo RUNTIME_INPUT=%RUNTIME_INPUT%

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

set SHARED_FLAG=OFF
if /I "%KIND%"=="Shared" set SHARED_FLAG=ON

set BUILD_DIR=%~dp0
set DEP_DIR=%BUILD_DIR%deps_windows_%ARCH%_!RUNTIME_FOR_CONFIG!_%CONFIG%_%SHARED_FLAG%
echo DEP_DIR=%DEP_DIR%

endlocal & exit /b 0

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