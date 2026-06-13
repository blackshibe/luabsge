# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Hard rules

- **Comments are banned entirely.** Do not write code comments — no `//`, no `/* */`, no doc comments, no "explanatory" comments, none. Existing comments may be left alone or deleted, but never add new ones. Explanations belong in chat, not in the source.
- The code is mid-port from OpenGL to Vulkan. Old/half-ported code may not compile; do not assume a green build. Only the renderer is changing — the Lua/sol2 and CMake layers are being reshaped around it.

## Build & run

This is developed on **Windows with MSVC + Ninja**. The `.sh` scripts are Unix-only and now hard-exit if run under Git Bash/MSYS — use the `.bat` equivalents:

```bat
setup_win.bat                 :: configure (calls vcvarsall x64, Ninja, C++20, compile_commands.json)
run_win.bat <project>         :: build (cmake --build) then run projects/<project>/luabsge.exe
run_win.bat vulkan/native_test
```

- `setup_win.bat` passes `-DFETCHCONTENT_UPDATES_DISCONNECTED=ON` so reconfiguring doesn't re-hit the network for already-fetched deps.
- The build **must** run inside the VS dev environment. If cl.exe reports missing `float.h`/`cstdint`/`stddef.h` etc., the MSVC `INCLUDE` paths aren't set — you ran `run.sh` instead of `run_win.bat`.
- There is no test suite. "Running" means launching one of the example projects in `projects/`.
- Unix/web builds exist (`setup.sh`/`run.sh`, `setup_web.sh`/`run_web.sh` via Emscripten) but are not the active path.

## C++ conventions

- **C++20** (`CMAKE_CXX_STANDARD 20`).
- **`src/` is an include root.** Use project-root-relative includes everywhere: `#include "engine/engine.h"`, `#include "util/time.h"`, `#include "lua/luax.h"`. Not `../`.

## Architecture

- **src/main.cpp / main.h** — entry point. `main.h` is an umbrella header (uses `// IWYU pragma: begin_exports`/`end_exports` to silence clangd's Include Cleaner).
- **src/engine/** — `EngineInstance`: owns the EnTT `registry`, the sol2 `lua` state, and the `unique_ptr<WindowInstance> window`. `main()` runs `EngineInstance()` → `preflight()` (sets up Lua, creates the window) → `start()`.
- **src/rendering/** — `WindowInstance` (base; owns the `GLFWwindow*` and the virtual `render_loop`/`render_pass`/`render_loop_init` + resize/focus callback hooks) and `VulkanWindowInstance`, which owns a `unique_ptr<VulkanRenderer>`.
- **src/rendering/vulkan/** — `vulkan_renderer.{h,cpp}` (`VulkanRenderer` holds the `Vulkan::Instance`/`Device`/`Swapchain` + surface) and `vulkan_bootstrap.{h,cpp}`, a hand-rolled **vk-bootstrap-style builder API** in `namespace Vulkan`: `InstanceBuilder`, `PhysicalDeviceSelector`, `DeviceBuilder`, `SwapchainBuilder` each return `std::optional<...>` from `build()`/`select()` (errors are optionals, not exceptions). Plain structs (`Instance`, `Device`, `Swapchain`) hold the raw handles; free `destroy_*` functions tear them down.
- **src/lua/** — `luax.{h,cpp}` (`Lua::util::run_script` using sol2's `safe_script_file`). `lib/` holds free Lua C-functions in namespaces (`Lua::global` for `print`/`warn`/`now`) — **no** `lua_` prefix. `class/` holds sol2 object/usertype bindings (e.g. `Lua::object::window::init(lua)`).
- **src/util/** — `output.h` (the `Output` logger, ANSI colors + filename via `std::source_location`), `time.h` (`Lua::time::now()`).
- **src/include/** — vendored imgui, implot, stb, `colors.h` (ANSI macros).
- **projects/** — each has `config.lua` (engine config) and `entry.lua` (game entry); assets in `mesh/`, `image/`, `shader/`, `font/`.

## Dependencies (all FetchContent, from source)

GLFW, Lua (walterschell), FreeType, Assimp, sol2 (`SOL_ALL_SAFETIES_ON`), GLM, Jolt, EnTT, Vulkan-Headers, volk, VMA. Native exe in `build/`; web output in `public/`.
