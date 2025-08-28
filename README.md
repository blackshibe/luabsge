# Luabsge

Small rendering engine that integrates Lua with OpenGL.<br/>

| test                                                  | raytracer                                                |  webtest                                                  |
| ----------------------------------------------------- | -------------------------------------------------------- |  ----------------------------------------------------- | 
| ![Test project](repo/luabsge.png)                          | ![Ray tracer](repo/luabsge_2.png)                   |  ![Test project](repo/webtest.png)                     |

## dependencies

-   glm
-   glfw
-   freetype2
-   imgui
-   assimp
-   sol2

## compilation

run `./run.sh test` to run the test project. Tested on Windows and Linux.

## features

-   OpenGL window drawing
-   GLM bindings
-   Mesh and texture loading
-   ASCII Font rendering
-   Post process shader effects
-   Ray tracing TBO support
-   ImGUI support

## todo

-   Showstoppers
    -   Abstracting objects properly
        -   Transform component
        -   Design a better abstraction layer
    -   The various TODOs
    -   Datamodel format
        - Scene hierarchy
        - Scene explorer
    -   Editor
        -   Scene creation format for a datamodel
    -   Clean the engine up
        -   Inconsistent function calling convention
        -   Inconsistent structure
        -   Don't use source built libs in native build
    -   Audio
-   Other
    -   Raytracer BVH
    -   UTF-8 Text Rendering

## goals

-   To have fun
