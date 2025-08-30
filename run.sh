if [ -z "$1" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else
	echo "[run.sh] compiling"
	
	# run make and don't run program if it errors
	if cmake --build build; then
		echo "[run.sh] compiled"
	else
		echo "[run.sh] failed to compile"
		exit
	fi

	echo "[run.sh] running"

	ROOTPATH=$(realpath .)
	cd projects/$1
	if [[ "$OSTYPE" == "win32" ]] || [[ "$OSTYPE" == "msys" ]]; then
		$ROOTPATH/build/Debug/luabsge.exe
	else 
		$ROOTPATH/build/luabsge
	fi
	cd ../../
fi