if [ -z "$1" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else
	echo "[run.sh] compiling"
	make   

	echo "[run.sh] running"

	# todo: fix
	cd projects/$1
	../../bin/program
	cd ../../
fi

