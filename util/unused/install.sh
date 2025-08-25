# i don't know how to use makefiles
export LD_LIBRARY_PATH=/usr/local/lib 

echo "[run.sh] compiling"
make   

echo "[run.sh] installing for $(whoami)"
echo "make sure ~/.local/bin is in your PATH!"
mv luabsge ~/.local/bin
