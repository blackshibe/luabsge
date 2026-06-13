# Regenerate filesystem without rebuilding the wasm module

# ./run_web_update_files.sh main/blackshibe /home/blackshibe/Downloads/blackshibe.github.io

if [ -z "$2" ]
  then
    echo "Usage: ./run_web_update_files.sh <project> <output_dir>"
else

	echo "-- Regenerating filesystem for projects/$1"
	$HOME/.emsdk/upstream/emscripten/tools/file_packager.py \
		public/luabsge.wasm.data \
		--js-output=public/luabsge.wasm.data.js \
		--preload projects/$1@/ \
		--use-preload-cache \
		--from-emcc

	mv public/luabsge.wasm.data $2/luabsge.wasm.data -f
	cp public/luabsge.wasm.data.js $2/luabsge.wasm.data.js
	echo "-- Done"
fi
