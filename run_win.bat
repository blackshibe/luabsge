@echo off
if not defined VSCMD_ARG_TGT_ARCH (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if "%~1"=="" (
    echo No project to run
    echo Specify the name of a folder inside ./projects as the first argument
    echo Ex. test
    exit /b 1
)

echo [run_win.bat] compiling
cmake --build build -j 16
if errorlevel 1 (
    echo [run_win.bat] failed to compile
    exit /b 1
)

echo [run_win.bat] compiled
echo [run_win.bat] running
cd projects\%~1
%~dp0build\luabsge.exe
cd %~dp0
