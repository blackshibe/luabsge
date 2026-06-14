case "$(uname -s)" in
	MINGW*|MSYS*|CYGWIN*)
		echo "[setup.sh] this is the Unix script; on Windows use setup_win.bat"
		exit 1
		;;
esac

cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -G Ninja