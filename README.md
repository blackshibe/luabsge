# LuaBSGE

Small cross-platform game engine that uses Lua as the scripting frontend.<br/>

| test                              | raytracer                         | webtest                           |
| --------------------------------- | --------------------------------- | --------------------------------- |
| ![Test project](repo/luabsge.png) | ![Ray tracer](repo/luabsge_2.png) | ![Test project](repo/webtest.png) |

## Dependencies

Everything is compiled from source.

-   glm
-   glfw
-   freetype2
-   imgui
-   assimp
-   sol2

## Compilation

```bash
# Native
./setup.sh
./run.sh minimal/native_test

# Web
./setup_web.sh minimal/web_test minimal/web_test
./run_web.sh minimal/web_test minimal/web_test # Path to a folder that will host the wasm files
```

## Features

-   OpenGL window drawing
-   GLM bindings
-   Mesh and texture loading
-   ASCII Font rendering
-   Post process shader effects
-   Ray tracing TBO support
-   ImGUI support

## TODO

-   Showstoppers
    -   Abstracting objects properly
        -   Transform component
        -   Design a better abstraction layer
    -   The various TODOs
    -   Datamodel format
        -   Scene hierarchy
        -   Scene explorer
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
