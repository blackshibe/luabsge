if [ -z "$1" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else
	echo "[run.sh] running"

	cd projects/$1
	../../luabsge
	cd ../../
fi

