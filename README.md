# LuaBSGE

Vulkan game engine with Lua backend

## TODO

- Better abstraction for images
- Sensible namespacing for engine elements
- Skybox
- Lighting

# Placeholder roadmap

## Renderer

- [ ] Window resizing doesn't even work
- [ ] Add normals and a material (metal/rough/AO/emissive) target to the G-buffer Reconstruct world position from depth instead of storing it
- [ ] Build a material system: base color, metal/rough factors, texture slots Bind it per draw instead of the single hardcoded texture
- [ ] Actually connect GLTF materials to meshes Load tangents Stop dropping every mesh but the first on a node
- [ ] Per-frame camera UBO (view/proj/cam pos/time)
- [ ] Compose parent transforms each frame The render loop currently pushes each node's local transform straight to the GPU and ignores parenting entirely, so anything under a parent draws in the wrong place
- [ ] there's zero cleanup for pipelines

## Lighting and fog

- [ ] Rewrite the lighting compute shader with real PBR (Cook-Torrance GGX) Start with one directional sun light
- [ ] Light components, collected into a buffer each frame (directional/point/spot)
- [ ] Ambient/IBL so the dark side of objects isn't pure black Constant ambient first, cubemap later
- [ ] Atmospheric fog Depth-based height fog as a fullscreen pass to start Maybe a real sky/scattering model later
- [ ] Shadows At least one shadow map, cascaded if we go outdoor
- [ ] Tonemapping and gamma before the swapchain copy, or the HDR target just clips

## Engine basics (needed before any gameplay)

- [ ] Per-frame update callback in Lua (on_update(dt)) There's no way to run game logic right now
- [ ] Input to Lua GLFW events are already polled, we just throw them away Wire keyboard/mouse/gamepad through
- [ ] Expose dt
- [ ] Spawn and destroy entities from Lua at runtime
- [ ] A free-fly / orbit camera controller as the first thing that uses input
- [ ] Instanced drawing A roguelike spawns tons of repeated tiles and props
- [ ] Shader hot-reload so iterating on lighting isn't painful

## Roguelike gameplay

- [ ] Wire up Jolt Physics component, fixed timestep, raycasts and movement exposed to Lua The old commented-out binding in ecs/instancecpp is a starting point
- [ ] Generate and upload meshes from Lua at runtime, not just import GLTF
- [ ] Stream rooms/chunks in and out without hitching
- [ ] Save/load the entity registry (also needed for netcode later)
- [ ] Audio Pick a library (miniaudio probably) SFX and positional sound
- [ ] Gameplay UI ImGui is fine for the prototype

## Multiplayer

- [ ] Decide the model before writing any of it Probably server-authoritative with a shared seed so everyone generates the same dungeon
- [ ] Pick a transport (ENet or GameNetworkingSockets) and split client/server
- [ ] Send inputs to the server, broadcast authoritative state back, interpolate on the client
- [ ] Networking API for Lua (RPCs, replicated components)

## Misc

- Write this README
- [ ] Make the camera a real engine concept, not just whichever imported node happens to be first
- [ ] Free-list for the GPU mesh/texture arrays once things get created and destroyed at runtime
- [ ] Frustum culling
- [ ] Recreate the draw image and G-buffer on window resize

## Autocomplete

- use "Extension Pack for C/C++" from KylindeTeam

## Compilation

```bash
# Native (MSYS2 UCRT64: gcc + Ninja)
/buildsh

# Run from inside the project folder
(cd projects/vulkan/native_test && ///build/luabsgeexe)
```
