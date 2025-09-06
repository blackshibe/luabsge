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

    -   The various TODOs

        -   Throw error when setting no mesh
        -   Patching ECS entities at runtime (mesh.color?)
        -   Queried mouse position doesn't match up with window position
        -   Camera matrix is only sent to the shader once per frame, and afaik right here it's already out of date by 1 frame
            -   Fixed?
        -   Remove all remaining functionality from Image and Texture besides loading them
        -   Entire engine breaks if BSGE isn't declared
        -   Can't use separate cameras for separate buffers(?)
        -   Buffer:bind(function() end)
        -   mesh_render is a mess

    -   Datamodel format
        -   Scene explorer
    -   Editor
        -   Scene creation format for a datamodel
    -   Clean the engine up
        -   Inconsistent function calling convention
        -   Inconsistent structure
        -   Don't use source built libs in native build
    -   Audio

-   Other
    -   UI Rendering hierarchy + how it's gonna interact with 3D objects
        -   New component for UI objects that has helpers for positioning
        -   Maybe similiar to Unity rendering?
    -   Raytracer BVH
    -   UTF-8 Text Rendering
