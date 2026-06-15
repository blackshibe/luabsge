#!/bin/bash

# ensure gcc exists, ensure CMake is configured, ensure build doesn't have missing globs, build

cd "$(dirname "$0")" || exit 1

rebuild=0
for arg in "$@"; do
	case "$arg" in
		-r | --rebuild) rebuild=1 ;;
		*)
			echo "[build.sh] unknown argument: $arg"
			echo "usage: build.sh [-r|--rebuild]"
			exit 1
			;;
	esac
done

setup() {
	command -v gcc >/dev/null 2>&1 || {
		echo "[build.sh] gcc not found on PATH"
		echo "Install MinGW-w64 (MSYS2 UCRT64 recommended) and add its bin dir to PATH"
		echo "e.g. C:\\msys64\\ucrt64\\bin"
		return 1
	}

	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
}

if [ "$rebuild" -eq 1 ]; then
	echo "[build.sh] rebuild requested, rebuilding cmake"
	setup
fi


if [ ! -f build/CMakeCache.txt ]; then
	echo "[build.sh] not configured, configuring"
	setup || exit 1
fi

log="/tmp/luabsge_build.log"
rm -f "$log"

echo "[build.sh] building"
cmake --build build -j "$(nproc)" 2>&1 | tee "$log"
build_err=${PIPESTATUS[0]}

[ "$build_err" -eq 0 ] && exit 0

# if globs need rebuilding then it rebuilds them
if ! grep -q "no known rule to make it" "$log"; then
	echo "[build.sh] failed to compile"
	exit 1
fi

echo "[build.sh] stale CMake glob detected (added/moved/deleted source), reconfiguring"
setup || exit 1
./build.sh
