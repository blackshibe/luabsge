CFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -llua -lfreetype -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp

SRC = src/glad/glad.c src/main.cpp src/opengl/*.cpp src/lua/*.cpp src/lua/module/*.cpp src/lua/class/*.cpp
OBJ = *.o

all: compile link

compile:
	ccache g++ -c $(SRC) 

link:
	ccache g++ $(CFLAGS) $(OBJ) -o luabsge $(LDFLAGS)