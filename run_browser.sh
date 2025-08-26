	# run make and don't run program if it errors
	# https://gist.github.com/ericoporto/8ce679ecb862a12795156d31f3ecac49
	echo "-- Running emsdk activate"
	emsdk activate latest

	echo "-- Running emsdk_env.sh"
	source $HOME/emsdk/emsdk_env.sh

	echo "-- Running cmake"
	cmake -B build-web -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ZLIB=ON -DUSE_EMSCRIPTEN=ON -DPLATFORM=Web -DEMSDK=$EMSDK -DCMAKE_TOOLCHAIN_FILE=~/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

	echo "-- Running build"
	if cmake --build build-web; then
		echo "[run.sh] compiled"
	else
		echo "[run.sh] failed to compile"
		exit
	fi
fi