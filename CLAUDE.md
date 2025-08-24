# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LuaBSGE (Lua-Based Small Game Engine) is a lightweight game engine that combines C++ OpenGL rendering with Lua scripting. The engine uses Sol2 for seamless C++/Lua integration and provides a complete API for 3D game development.

## Build Commands

```bash
# Build and run a project
./run.sh test

# Debug build with GDB
./debug.sh test

# Install to ~/.local/bin
./install.sh
```

The build system uses GNU Make and compiles to `bin/program`. Uses ccache for faster compilation.

## Dependencies (Arch Linux)

- OpenGL, GLFW, GLM (graphics)
- FreeType2 (text rendering) 
- Lua, Sol2 (scripting)
- Assimp (3D model loading)

## Architecture

### Core Structure
- **C++ Engine Core** (`src/`): OpenGL rendering, window management, asset loading
- **Lua Bindings** (`src/lua/`): Sol2-based bindings for engine systems
- **Project System** (`projects/`): Each project is a folder with `entry.lua` as entry point

### Key Directories
- `src/lua/class/`: Lua-bindable C++ classes (Camera, Font, Image, Mesh, etc.)
- `src/lua/module/`: System modules (rendering, UI, GLM math)
- `src/opengl/`: OpenGL rendering pipeline and utilities
- `projects/[name]/`: Self-contained game projects with assets

### Lua API Classes
- **Camera**: 3D camera with FOV, position, near/far clipping
- **Font/Textlabel**: Text rendering system
- **Image**: Texture loading and management
- **Mesh**: 3D model loading with Assimp
- **Signal**: Event system for callbacks
- **World.rendering**: Main render loop and camera management

## Development Patterns

### Sol2 Binding Format
All Lua bindings follow this pattern (see `font.cpp` as reference):
```cpp
void lua_bsge_init_[class](sol::state &lua) {
    lua_State *L = lua.lua_state();
    auto method = [&L](Class *obj, params...) {
        // error handling with luax_push_error(L, "message")
        return result;
    };
    
    lua.new_usertype<Class>("ClassName", 
                           "method", method);
}
```

### Project Structure
Each project in `projects/[name]/` contains:
- `entry.lua`: Main entry point
- `font/`, `image/`, `mesh/`, `shader/`: Asset directories

### Error Handling
- C++ functions return 0 for success, non-zero for failure
- Lua errors use `luax_push_error(L, "message")` in lambda wrappers
- Sol2 bindings handle Lua stack management automatically

## Current Development State

### Working Systems
- OpenGL 4.0 Core Profile rendering
- 3D model loading and rendering
- Texture and font loading
- Complete GLM math bindings
- Camera and render loop systems

### Known Issues
- Text rendering has positioning problems
- Text scaling requires window resize to work properly
- ImGui integration is disabled

## Testing

Run the test project to verify engine functionality:
```bash
./run.sh test
```

The test project demonstrates loading textures, 3D meshes, fonts, and basic rendering.