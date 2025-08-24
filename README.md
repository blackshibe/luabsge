# Luabsge

Small rendering engine that integrates Lua with OpenGL.<br/>

![screenshot](luabsge.png)

## dependencies (archlinux)

-   glm
-   glfw
-   freetype2
-   imgui
-   assimp
-   sol2

## compilation

run `./run.sh test` to run the test project

note: LuaBSGE is only tested on Linux right now

## todo

-   Meshes don't render with non-RT
-   Create a universal class constructor
-   Make creating new classes easier
-   Text rendering has really weird positioning
-   Text rendering cuts off with high scale (you need to resize the window once to fix it)
