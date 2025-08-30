# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

LuaBSGE is a cross-platform game engine with C++ backend and Lua scripting frontend. It combines:
- **C++ Engine Core**: OpenGL rendering, physics (Jolt), asset loading (Assimp), font rendering (FreeType)
- **Lua Scripting Layer**: Game logic, scene management via sol2 bindings
- **ECS Architecture**: Entity-Component-System using EnTT registry
- **Cross-Platform**: Native builds (Linux/Windows) and WebAssembly via Emscripten

## Common Commands

### Native Development
```bash
# Initial setup
./setup.sh

# Build and run a project
./run.sh <project_name>
# Examples:
./run.sh minimal/native_test
./run.sh main/blackshibe
```

### Web Development
```bash
# Setup for web build (requires emsdk)
./setup_web.sh <project_name> <output_directory>

# Build and deploy to web
./run_web.sh <project_name> <output_directory>
# Example:
./setup_web.sh minimal/web_test minimal/web_test
./run_web.sh minimal/web_test minimal/web_test
```

### Manual Build Commands
```bash
# Native build
cmake -B build
cmake --build build

# Web build (with emsdk activated)
cmake -B build-web -DPROJ=<project> -DUSE_EMSCRIPTEN=ON -DCMAKE_TOOLCHAIN_FILE=~/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
cmake --build build-web
```

## Architecture

### Core Structure
- **src/main.cpp**: Entry point, initializes GLFW, Lua state, and engine systems
- **src/lua/**: Lua-C++ bindings and scripting interface
  - **lua.cpp**: Main Lua state initialization and bindings setup
  - **class/**: C++ classes exposed to Lua (Mesh, Camera, Shader, etc.)
  - **module/**: Lua modules (rendering, input, ImGui bindings)
  - **ecs/**: Entity-Component-System implementation with EnTT
- **src/opengl/**: OpenGL rendering utilities (shaders, window management)
- **src/physics/**: Jolt physics integration

### ECS System
The engine uses EnTT for entity-component management:
- **BSGEObject**: Wrapper around entt::entity with transform and parent hierarchy
- **Components**: MeshComponent, TextureComponent, PhysicsComponent
- **Registry**: Global static registry accessible via `lua_bsge_get_registry()`

### Lua Integration
- **sol2**: Modern C++ Lua binding library
- **Classes**: C++ objects exposed as Lua usertypes (Mesh, Camera, Image, etc.)
- **World Table**: Global Lua table containing engine state and systems
- **Projects**: Located in `projects/` directory, each with `config.lua` and `entry.lua`

### Project Structure
Each project in `projects/` contains:
- **config.lua**: Engine configuration (window size, title, etc.)
- **entry.lua**: Main game script entry point
- **Assets**: Organized in subdirectories (mesh/, image/, shader/, font/)

### Build System Notes
- **CMake**: Primary build system with FetchContent for dependencies
- **Cross-compilation**: Same codebase builds for native and web via USE_EMSCRIPTEN flag
- **Dependencies**: All fetched from source (GLFW, Lua, FreeType, Assimp, sol2, GLM, Jolt, EnTT)
- **Output**: Native executable in `build/`, web files in `public/`

## Development Workflow

1. Create new project directory in `projects/`
2. Add `config.lua` and `entry.lua` files
3. Use `./run.sh <project_name>` for iterative development
4. For web deployment, use `./setup_web.sh` and `./run_web.sh`

## Key Files for Understanding
- **src/main.cpp**: Engine initialization sequence
- **src/lua/lua.cpp**: Lua binding setup and class registration
- **src/lua/ecs/object.h**: ECS object structure
- **projects/main/refactor/entry.lua**: Example of modern Lua scripting patterns