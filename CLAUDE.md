# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Comments

Comments are banned

## What this is

LuaBSGE is a Vulkan game engine with a Lua scripting frontend. The C++ side is engine plumbing; actual "games" live in `projects/` as Lua scripts plus assets — the engine itself ships no default content.

The codebase is **mid-port from OpenGL to Vulkan**. Old/half-ported code may not compile; do not assume a green build. The active path is the Vulkan renderer; `oldsrc/` and `projects/opengl/` are legacy. The Vulkan renderer closely follows the structure of [vkguide.dev](https://vkguide.dev).

## Build & run

Developed on **Windows with MSYS2 UCRT64 (gcc + Ninja + GDB)**. `build.sh` is the single entry point — it configures on first run (or after `build/` is deleted) and builds:

```bash
./build.sh        # configure if needed, then build (Debug, gcc/g++, Ninja, compile_commands.json)
```

- `gcc` must be on PATH (e.g. `C:\msys64\ucrt64\bin`). `build.sh` errors out if it isn't.
- It auto-detects stale CMake globs (added/moved/deleted source files) and reconfigures, since sources are picked up via `file(GLOB_RECURSE src/*.cpp)`.
- Reconfigure uses `-DFETCHCONTENT_UPDATES_DISCONNECTED=ON` so already-fetched deps don't re-hit the network.
- The executable is built to `build/luabsge.exe`.
- There is **no test suite**. "Running" means launching one of the `projects/` examples.

### Running a project

The engine loads `config.lua` then `entry.lua` **relative to the current working directory** (there is no `chdir` in C++ and no project-selection arg). So you must run the binary from inside the project directory, e.g.:

```bash
cd projects/vulkan/native_test && ../../../build/luabsge.exe
```

## C++ conventions

- **C++20**, `src/` is an include root: use project-relative includes everywhere (`#include "engine/engine.h"`, `#include "lua/luax.h"`), never `../`.
- Formatting is enforced by `.clang-format` (LLVM base, **tabs** for indentation, width 4, no column limit, attached braces, right-aligned pointers/refs, `NamespaceIndentation: All`). Run clang-format on changed files.
- Errors in the Vulkan bootstrap layer are returned as `std::optional`, not thrown.

## Architecture

- **`src/main.cpp`** — entry point. Constructs `EngineInstance` → `preflight()` → `start()`, wrapped in a try/catch.
- **`src/engine/`** — `EngineInstance` owns the three core pieces: the EnTT `registry`, the sol2 `lua` state, and `unique_ptr<WindowInstance> window`. The constructor registers all Lua bindings and runs `config.lua`; `preflight()` creates the `VulkanWindowInstance`, sets up ImGui/ImPlot, and runs `entry.lua`; `start()` enters the render loop. Also `queue.{h,cpp}` (`DeletionQueue` for ordered Vulkan teardown).
- **`src/rendering/window/`** — `WindowInstance` (base; owns `GLFWwindow*` and virtual `render_loop`/`render_pass`/`render_loop_init` + resize/focus callbacks) and `VulkanWindowInstance`, which owns a `unique_ptr<Vulkan::Renderer>`.
- **`src/rendering/vulkan/`**
    - `base/` — bootstrap (instance/device/swapchain builders), VMA setup (`vulkan_vma.cpp`), images, descriptors, and `vulkan_init.*` / `vulkan_types.h` helpers.
    - `pipeline/` — `ComputePipeline`, `GraphicsPipeline`, and shared shading helpers.
    - `renderer/` — `Vulkan::Renderer` implementation split across `_init`, `_rendering`, and `_buffer` translation units. Renders into an off-swapchain RGBA16F `draw_image`, then copies to the swapchain. Double-buffered (`FRAME_OVERLAP = 2`), with per-frame and lifetime `DeletionQueue`s.
- **`src/ecs/`** — `instance.h` defines the ECS data: `Ecs::Instance` (name, `glm::mat4` transform, parent entity) stored in the EnTT registry. `scene_mesh.h` / `scene_camera.h` for mesh and camera components.
- **`src/lua/`** — `luax.{h,cpp}` (`Lua::util::run_script`, sol2 `safe_script_file`). `lib/` holds the binding modules, each with an `init(...)` that registers a global table or usertype: `lua_global` (`print`/`warn`/`now`), `lua_imgui`, `lua_instance` (`Instance` usertype → creates EnTT entities), `lua_gltf` (`GLTF.import`). `class/window.*` binds the window. Bindings live in `Lua::*` / `Lua::object::*` namespaces — **no `lua_` symbol prefix**.
- **`src/util/`** — `output.h` (`Output` logger: ANSI colors + source location), `time.{h,cpp}` (`now()`).
- **`src/include/`** — vendored imgui, implot, stb, `colors.h`.

## Lua API & type generation

Lua bindings carry Doxygen-style doc comments (`@namespace`/`@class`, `@field`, optional `@param`/`@return`) directly above the sol2 registration. `tools/gen_types.py` scans these and emits LuaCATS annotations for the sumneko/LuaLS language server:

```bash
python tools/gen_types.py -o projects/types.d.lua   # defaults to scanning src/lua/lib/*.cpp
```

When adding or changing a Lua binding, update its doc comment and regenerate `projects/types.d.lua` so editor completion stays in sync. (`document.sh` + `util/document.py` are the older, removed version of this tooling.)

## Project structure

Each project under `projects/<renderer>/<name>/` has:

- `config.lua` — engine config (sets the `BSGE` table: asset dir, `WindowConfiguration`). Run first.
- `entry.lua` — game entry point. Run second.
- `shader/` — GLSL `.vert`/`.frag`/`.comp`. CMake compiles every shader under `projects/` to `.spv` via glslang (target `vulkan1.3`) as part of the build.
- `asset/` — meshes (`.glb`/`.blend`), images, etc.

## Dependencies

All via CMake `FetchContent` from source: GLFW, Lua (walterschell), Assimp, sol2 (`SOL_ALL_SAFETIES_ON`), GLM, Jolt physics, EnTT, Vulkan-Headers, volk (loader; `VK_NO_PROTOTYPES`), VulkanMemoryAllocator, glslang. An Emscripten/WASM path exists in `CMakeLists.txt` (`-DUSE_EMSCRIPTEN`, output to `public/`) but is not the active target.
