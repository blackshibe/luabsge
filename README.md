# Luabsge

Small rendering engine that integrates Lua with OpenGL.<br/>

![screenshot](luabsge.png)
![screenshot](luabsge_2.png)

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

-   RT Test improvements
-   Audio
-   Physics
-   UTF-8 Rendering
-   Create a universal class constructor
-   Text rendering has really weird positioning
-   Text rendering cuts off with high scale (you need to resize the window once to fix it)
