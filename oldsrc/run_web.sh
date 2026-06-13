# this copies build files to an external folder to run them

# ./run_web.sh minimal/webtest minimal/webtest
# ./run_web.sh main/blackshibe /home/blackshibe/Downloads/blackshibe.github.io

if [ -z "$2" ]
  then
    echo "No project to run"
else

	echo "-- Running build"
	if cmake --build build-web -j 16; then
		echo "[run.sh] compiled"

		echo "-- Packaging files"
		$HOME/.emsdk/upstream/emscripten/tools/file_packager.py \
			public/luabsge.wasm.data \
			--js-output=public/luabsge.wasm.data.js \
			--preload projects/$1@/ \
			--use-preload-cache \
			--from-emcc

		mv public/luabsge.wasm.data $2/luabsge.wasm.data
		cp public/luabsge.wasm.data.js $2/luabsge.wasm.data.js
		cp public/luabsge.wasm.js $2/luabsge.wasm.js
		cp public/luabsge.wasm.wasm $2/luabsge.wasm.wasm
	else
		echo "[run.sh] failed to compile"
		exit
	fi
fi
