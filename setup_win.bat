@echo off
if not defined VSCMD_ARG_TGT_ARCH (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
REM FETCHCONTENT_UPDATES_DISCONNECTED skips the per-configure git "update" step
REM for deps already present in build/_deps, so reconfiguring doesn't need network
REM for cached deps (only first-time downloads hit the network).
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -G Ninja