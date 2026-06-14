@echo off
where gcc >nul 2>&1 || (
    echo [run_win.bat] gcc not found on PATH
    echo Install MinGW-w64 ^(MSYS2 UCRT64 recommended^) and add its bin dir to PATH
    exit /b 1
)

if "%~1"=="" (
    echo No project to run
    echo Specify the name of a folder inside ./projects as the first argument
    echo Ex. test
    exit /b 1
)

echo [run_win.bat] compiling
cmake --build build -j 16 > "%TEMP%\luabsge_build.log" 2>&1
set BUILD_ERR=%errorlevel%
type "%TEMP%\luabsge_build.log"

if %BUILD_ERR% equ 0 goto compiled

findstr /c:"no known rule to make it" "%TEMP%\luabsge_build.log" >nul
if errorlevel 1 (
    echo [run_win.bat] failed to compile
    exit /b 1
)

echo [run_win.bat] stale CMake glob detected ^(moved/deleted source^), reconfiguring
call "%~dp0setup_win.bat"

echo [run_win.bat] recompiling
cmake --build build -j 16
if errorlevel 1 (
    echo [run_win.bat] failed to compile after reconfigure
    exit /b 1
)

:compiled
echo [run_win.bat] compiled
echo [run_win.bat] running
cd projects\%~1
%~dp0build\luabsge.exe
cd %~dp0
