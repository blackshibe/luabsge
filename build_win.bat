@echo off

REM complete build process:
REM ensure gcc exists, ensure CMake is configured, ensure build doesn't have missing globs, build, run

where gcc >nul 2>&1 || (
    echo [build_win.bat] gcc not found on PATH
    echo Install MinGW-w64 ^(MSYS2 UCRT64 recommended^) and add its bin dir to PATH
    exit /b 1
)

if not exist "%~dp0build\CMakeCache.txt" (
    echo [build_win.bat] not configured, configuring
    call :configure
    if errorlevel 1 exit /b 1
)

echo [build_win.bat] building
cmake --build build -j 16 > "%TEMP%\luabsge_build.log" 2>&1
set BUILD_ERR=%errorlevel%
type "%TEMP%\luabsge_build.log"

if %BUILD_ERR% equ 0 exit /b 0

findstr /c:"no known rule to make it" "%TEMP%\luabsge_build.log" >nul
if errorlevel 1 (
    echo [build_win.bat] failed to compile
    exit /b 1
)

echo [build_win.bat] stale CMake glob detected ^(added/moved/deleted source^), reconfiguring
call :configure
if errorlevel 1 exit /b 1

echo [build_win.bat] recompiling
cmake --build build -j 16
exit /b %errorlevel%

:configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
exit /b %errorlevel%
