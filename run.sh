if [ -z "$1" ]
  then
    echo "No project to run"
    echo "Specify the name of a folder inside ./projects as the first argument"
    echo "Ex. test"
else
	# i don't know how to use makefiles
	export LD_LIBRARY_PATH=/usr/local/lib 

	echo "[run.sh] compiling"
	make   

	# mv luabsge ~/.local/bin

	echo "[run.sh] running"

	cd projects/$1
	../../luabsge
	cd ../../

	echo "[run.sh] cleanup"
	rm -f *.o
	rm -f luabsge
fi

