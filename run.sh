if [ -z "$1" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else
	echo "[run.sh] compiling"

	# run make and don't run program if it errors
	if make; then
		echo "[run.sh] compiled"
	else
		echo "[run.sh] failed to compile"
		exit
	fi

	echo "[run.sh] running"

	# todo: fix
	cd projects/$1
	../../bin/program
	cd ../../
fi

