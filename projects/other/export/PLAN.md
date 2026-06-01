# Plan: full `.rbxl` → asset folder + datamodel snapshot, without Roblox

## Goals

1. Run with no Studio, no Roblox client — pure CLI on the `.rbxl` file at `in/Exporter.rbxl`.
2. Produce a deterministic `out/` tree containing: a JSON datamodel, every script's source on disk, and every referenced asset (meshes, textures, sounds, animations, decals, skyboxes, font files, anything reachable through a `rbxassetid://` literal — including ones that only appear inside `Script.Source`).
3. Decode what we can: meshes via the C++ `filemesh` tool; animations from raw `rbxm` to JSON keyframes; everything else stored raw with the right extension.

## Tooling

- **Lune** ([lune-org/lune](https://github.com/lune-org/lune)) handles everything except mesh decoding:
  - `@lune/roblox` wraps `rbx-dom` and exposes the same instance API the existing `export.lua` already uses (`GetChildren`, `FindFirstChild`, `instance.ClassName`, property access). `roblox.deserializePlace(fs.readFile("in/Exporter.rbxl"))` returns a DataModel handle.
  - `@lune/net` does the asset HTTP downloads (with cookie auth and binary response bodies).
  - `@lune/fs` for filesystem I/O, `@lune/serde` for JSON, `@lune/process` to invoke the mesh decoder.
- **C++ `filemesh`** for mesh → OBJ (already built). Spawned from Lune via `process.spawn`.

No Python, no Studio, no Roblox client — one Lune binary plus one native exe.

## Output layout

```
out/
  datamodel.json              # full instance tree + properties
  manifest.json               # asset_id -> local file mapping (importer reads this)
  assets.csv                  # one row per (asset_id, kind, first-ref name) — input to downloader
  scripts/<instance path>.luau   # every Script/LocalScript/ModuleScript .Source
  mesh/<asset_id>.mesh        # raw, then sibling .obj from filemesh
  image/<asset_id>.<ext>      # png/jpg/webp/gif by magic bytes
  sound/<asset_id>.<ext>      # ogg/mp3
  animation/<asset_id>.rbxm   # raw, plus <asset_id>.keyframes.json (decoded)
  font/<asset_id>.<ext>
```

## Phase 1 — `export.luau` (Lune): place-file ingest

Replace the Studio-bound `export.lua` with a Lune script:

```lua
local roblox = require("@lune/roblox")
local fs     = require("@lune/fs")
local game   = roblox.deserializePlace(fs.readFile("in/Exporter.rbxl"))
```

Walk every service under `game`. The current `funcs` table (handlers for `Attachment` / `MeshPart` / `Weld` only) is too narrow — generalize to:

- **Always-on properties** for every instance: `Name`, `ClassName`, plus `CFrame`/`Transform` when present (read via `pcall` so non-CFrame instances don't fault).
- **Per-class allowlist** stored in a single Lua table:
  ```
  CFrameValue:   Value
  Camera:        CFrame, FieldOfView
  BasePart:      CFrame, Size, Color, Material, Transparency, Anchored, CanCollide
  MeshPart:      + MeshId, TextureID
  SpecialMesh:   MeshId, TextureId, MeshType, Scale, Offset
  Decal/Texture: Texture, Face, StudsPerTileU, StudsPerTileV
  ImageLabel/ImageButton: Image
  Sound:         SoundId, Volume, RollOffMode, Looped
  Animation:     AnimationId
  ParticleEmitter / Beam / Trail: Texture (+ shape attrs)
  Sky:           SkyboxBk/Dn/Ft/Lf/Rt/Up
  Shirt/Pants:   ShirtTemplate / PantsTemplate
  ShirtGraphic:  Graphic
  MaterialVariant / SurfaceAppearance: ColorMap, NormalMap, MetalnessMap, RoughnessMap
  VideoFrame:    Video
  Script / LocalScript / ModuleScript: Source (handled separately, see phase 2)
  ```
- **Fallthrough**: for any string property whose value starts with `rbxassetid://` or `rbxasset://`, capture it even if the class isn't in the allowlist. Cheap insurance against rare classes.

Output of phase 1: in-memory tree mirroring the instance hierarchy, with each node `{ class_name, name, properties = {...}, children = {...} }`. Serialized to `out/datamodel.json`.

## Phase 2 — script source extraction

During the same walk, when an instance is `Script | LocalScript | ModuleScript`:

- Compute its full path (`Workspace/model/receiver/Test`).
- Write `instance.Source` to `out/scripts/<path>.luau` (rojo-style: containers with both children and source become a directory with `init.luau`).
- Keep `Source` *out* of `datamodel.json` — replace it with `{ source_file = "scripts/.../init.luau" }` so the JSON stays small.

## Phase 3 — asset reference collection

Two passes feed one deduplicated `assets` map keyed by numeric asset ID:

1. **Property pass** — every `rbxassetid://N` value encountered during the phase-1 walk is added. Record `{ kind = "mesh"|"image"|"sound"|"animation"|"font"|"video"|"unknown", refs = [{ instance_path, property_name }, ...] }`. `kind` is inferred from the property name (`MeshId` → mesh, `Image`/`Texture`/`ColorMap` → image, `SoundId` → sound, `AnimationId` → animation, …). For the fallthrough catch-all, `kind = "unknown"`; resolve later by magic bytes after download.
2. **Script-source pass** — for every `.Source` already written to disk, regex-scan with `rbxassetid://(%d+)`. Add hits with `kind = "unknown"` and `refs = [{ script_path }]`. Catches IDs that only appear in dynamic `SoundService:PlayLocalSound`, `ContentProvider:PreloadAsync({...})`, in-script asset lists, etc.

Write `out/assets.csv` (compatible with the current downloader's columns: `name,url` where `name = <kind>/<asset_id>` and `url = rbxassetid://<id>`) and `out/manifest.json` (richer — keeps the refs so the luabsge importer can wire instances to files).

## Phase 4 — downloader (`download.luau`)

A second Lune script reads `out/manifest.json` and does the downloads via `@lune/net.request`:

```lua
local net = require("@lune/net")
local res = net.request({
    url     = "https://assetdelivery.roblox.com/v1/asset/?id=" .. id,
    method  = "GET",
    headers = {
        ["User-Agent"] = "Roblox/WinInet",
        Cookie = ".ROBLOSECURITY=" .. cookie,
        Referer = "https://www.roblox.com/",
    },
})
-- res.body is binary; res.ok / res.statusCode for error handling.
fs.writeFile(target, res.body)
```

Logic per asset:
- Magic-byte sniff to decide extension:
  - `\x89PNG` → `.png`, `\xFF\xD8\xFF` → `.jpg`, `GIF8` → `.gif`, `RIFF…WEBP` → `.webp` (image)
  - `OggS` → `.ogg`, `ID3` or `\xFF\xFB`/`\xFF\xF3`/`\xFF\xF2` → `.mp3` (sound)
  - `<roblox!` → `.rbxm` (binary Roblox) — animations land here, decode in phase 5b
  - `<roblox` (plain `<`) → `.rbxmx` (XML Roblox)
  - Roblox FileMesh `version ` ASCII prefix → `.mesh`
- For rows with `kind == "unknown"`, infer kind from the sniffed extension and update the manifest entry.
- Re-entrant: if `out/<kind>/<id>.<ext>` already exists, skip — lets the pipeline resume after partial runs.
- Concurrency: Lune is single-threaded but `net.request` is async; fan out N requests at once with a small in-script semaphore (e.g. 8 in flight) to keep the CDN happy.
- `.cookie` file is read from the project root, same as before.

## Phase 5 — decoders

- **Meshes**: shell out to the C++ `filemesh.exe` per `out/mesh/*.mesh` so an `.obj` lands next to each.
- **Animations**: animation assets are themselves serialized `rbxm` files containing a `KeyframeSequence`. Open them with Lune (`roblox.deserializeModel`) and dump to JSON: `{ name, length, keyframes: [{ time, pose: { [bone_name]: { cframe, easing_style, easing_direction } } }] }`. Saved alongside the raw as `out/animation/<id>.keyframes.json`.
- **Images / sounds / fonts / videos**: leave raw — luabsge can consume them directly.

## Phase 6 — manifest finalize

After downloads + decodes, rewrite `out/manifest.json` so each entry has a final `file` field:

```json
{
  "14576660028": {
    "kind": "mesh",
    "file": "mesh/14576660028.obj",
    "raw":  "mesh/14576660028.mesh",
    "refs": [{ "instance": "Workspace/model/receiver", "property": "MeshId" }]
  }
}
```

This is what the luabsge importer reads — never the rbxl, never the CSVs.

## Driver

Replace `run.sh` with a sequenced pipeline:

```
1. lune run export.luau   in/Exporter.rbxl out/        # phases 1–3
2. lune run download.luau out/                          # phase 4 (spawns filemesh.exe per mesh in 5a)
3. lune run decode.luau   out/                          # phase 5b + 6 (animations, finalize manifest)
```

Or a single `run.luau` driver that calls the three in sequence — open question, defer to implementation time.

## Order of implementation

1. **Phase 1 + 2 (Lune)** — get `export.luau` parsing the rbxl, dumping `datamodel.json` and `scripts/`. This proves Lune works against the actual `Exporter.rbxl` and is the riskiest step; everything else is mechanical once the instance tree is in hand.
2. **Phase 3** — collect IDs, write `assets.csv` + initial `manifest.json`. Compare against the existing `in/meshes.csv` + `in/images.csv` as a sanity check (the new pass should be a strict superset).
3. **Phase 4** — switch `download_assets.py` to read the new CSV; add sound/rbxm sniffing.
4. **Phase 5a** — wire `filemesh.exe` into the driver.
5. **Phase 5b + 6** — animation decoder + manifest finalize. Smallest surface; do last.

## Risks / things to settle before coding

- **Restricted assets**: any asset that requires the place owner's session will still 403 even with a cookie if the running account doesn't have the right (this is the same risk the current downloader has). The fallthrough script-scan will surface assets the original script never even looked at — expect more 403s, not fewer. Manifest should record failure status per asset so the importer can warn instead of silently using a missing file.
- **Asset-kind inference for script-scanned IDs**: there's no reliable way short of downloading and sniffing. Plan is to default to `unknown` and resolve post-download — `download_assets.py` already half-does this for images.
- **Property dump completeness**: the per-class allowlist is the right tradeoff (small JSON, no garbage), but it will miss exotic properties. Worth keeping the allowlist as a single top-of-file table so new classes are a one-line add.
- **CFrame serialization**: keep the existing 12-float `GetComponents()` form — luabsge already consumes it.
