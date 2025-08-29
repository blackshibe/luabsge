# this copies build files to an external folder to run them

# ./run_browser.sh webtest /home/blackshibe/Downloads/luabsge/projects/webtest
# ./run_browser.sh blackshibe /home/blackshibe/Downloads/blackshibe.github.io

if [ -z "$2" || -z "$2" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else

	echo "-- Running build"
	if cmake --build build-web; then
		echo "[run.sh] compiled"

		mv public/luabsge.wasm.data $2/luabsge.wasm.data
		mv public/luabsge.wasm.js $2/luabsge.wasm.js
		mv public/luabsge.wasm.wasm $2/luabsge.wasm.wasm
	else
		echo "[run.sh] failed to compile"
		exit
	fi
fi

