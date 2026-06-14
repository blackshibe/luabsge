@echo off
where gcc >nul 2>&1 || (
    echo [setup_win.bat] gcc not found on PATH
    echo Install MinGW-w64 ^(MSYS2 UCRT64 recommended^) and add its bin dir to PATH
    echo e.g. C:\msys64\ucrt64\bin
    exit /b 1
)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
