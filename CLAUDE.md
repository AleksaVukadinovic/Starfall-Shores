# Starfall Shores

A C++23 / OpenGL 3.3 core graphics engine and demo scene, built for the Computer Graphics course at MATF (Faculty of Mathematics, Belgrade). The repo has two parts:

- `engine/`: a static library (`matf-rg-engine`) with the core loop, controllers, rendering, resources, sound, and utilities.
- `app/`: the executable (`APP`). It's a stylized island scene with a campfire, tents, lots of vegetation, water, a lighthouse, a day/night cycle, fog, rain, and bloom.

## Build & run

```bash
./setup.sh                                    # one-time: brew/apt deps (assimp, llvm, ninja...) + venv with libclang
cmake -B build -G Ninja && cmake --build build   # CLion uses cmake-build-debug/
cd app && ./APP                               # MUST run from app/
```

- The binary is written to `app/APP` (`RUNTIME_OUTPUT_DIRECTORY` is set to `app/`).
- **Working directory matters.** `config.json` and `resources/...` are resolved relative to the CWD. If `config.json` isn't found, `Configuration` silently **writes a default `config.json` into the CWD**. Pass `--configuration <path>` to override.
- Assimp comes from the system (`find_package(ASSIMP)`). The other libs are vendored in `engine/libs/` (glfw, glad, glm, spdlog, imgui, nlohmann json, stb, miniaudio).
- There are no tests. To verify a change, build it and run the app.
- Docs: `doxygen Doxyfile` → `docs/html/index.html`.

### Style / lint

- `python3 check.py app` or `check.py engine` runs on demand (needs `venv` with libclang). It enforces:
  - `namespace`/functions/params/locals: `snake_case`. Classes/structs/enums: `PascalCase`.
  - `static const`/`constexpr` vars: `UPPER_CASE`. Private class members: `m_` prefix. Globals: `g_` prefix.
  - Public/struct fields: `snake_case` (PascalCase is tolerated, e.g. `Camera::Position`).
  - No `<iostream>` and no relative `#include "../..."`.
  - **app code must not include glad/GLFW/assimp directly.** Go through the engine instead.
- Formatting: `.clang-format` (LLVM-based, 4-space indent, no column limit). Logging goes through `spdlog`, never iostream.
- Errors: `RG_GUARANTEE(cond, fmt, ...)`, `RG_SHOULD_NOT_REACH_HERE`, `RG_UNIMPLEMENTED`, `util::EngineError`. Wrap GL calls as `CHECKED_GL_CALL(glFn, args...)`.

## Architecture

### Main loop and controllers (`engine/core`)
`App::run` runs `engine_setup` → `app_setup` (user override) → `initialize` → loop { `poll_events`, `update`, `draw` } → `terminate`.

Everything is a **`Controller` singleton** that hooks into these phases: `initialize`, `loop`, `poll_events`, `update`, `begin_draw`, `draw`, `end_draw`, `terminate`.
- Register controllers in `MyApp::app_setup` with `register_controller<T>()`. Access them anywhere with `Controller::get<T>()`. Calling `get` on an unregistered controller fails loudly.
- Order comes from `a->before(b)` / `b->after(a)`, then a **topological sort** (cycles are an error). `EngineControllersBegin/End` are sentinels around the engine controllers (Platform → Graphics → Resources → Sound).
- `draw` phase order: every enabled controller's `begin_draw` in order, then every `draw` in order, then every `end_draw` in **reverse** order. `terminate` also runs in reverse. `poll_events` runs even for disabled controllers.

### Frame pipeline
1. `GraphicsController::begin_draw` clears the buffers.
2. `ShadowController::begin_draw` re-fits the shadow cascades around the camera and marks stale ones.
3. `Renderer::begin_draw`:
   - renders **only stale** shadow cascades by calling the depth-scene callback (`set_depth_scene`),
   - uploads the per-frame uniform buffer (`FrameUniforms`: camera, light, point lights, fog, cascade matrices),
   - builds the view frustum used for culling.
4. `PostProcessingController::begin_draw` binds the HDR FBO. It has 2 color attachments: scene and bright (bloom).
5. `MainController::draw` draws the scene through `Renderer`, then the skybox. `GUIController::draw` draws ImGui, into the HDR FBO.
6. `PostProcessingController::end_draw`: bloom mip chain (13-tap downsample and tent upsample over `BLOOM_MIP_COUNT` half-res mips), then `bloom_final.glsl`, which applies exposure, greyscale, underwater tint, and the rain overlay.
7. `GraphicsController::end_draw` closes the profiler frame and swaps buffers.

`GpuProfiler` (owned by `GraphicsController`, `graphics->profiler()`) times consecutive sections: Shadows, Scene, GUI, Post-processing. Results are shown in the Debug settings tab along with `Renderer::stats()` (draw calls, drawn and culled objects). Use it to measure the cost of every new effect.

### Rendering API (`engine/graphics/Renderer.hpp`, `DrawBatch.hpp`)
- `renderer->create_batch(model, span<TransformData>, DrawMethod)` returns a `const DrawBatch*`. Model matrices and world AABBs are computed once. `Instanced` batches own an `InstanceBuffer` (a VBO plus one VAO per mesh), so there is no GL object creation per frame. Batches are owned and destroyed by the Renderer.
- `draw(batch, shader, Effect)` frustum-culls per transform. Instanced batches upload only the visible matrices, orphaning the buffer. `Blended` enables alpha blending.
- `draw(model, shader, mat4, Effect)` is for one-off dynamic transforms (e.g. the placement tool).
- `draw_depth(batch)` is only valid inside the depth-scene callback. It picks `depth_shader` or `depth_instanced_shader` itself.
- Resolve `Model*`/`Shader*` once at init. Never look them up by string per frame.
- `Effect::{None, Wind, Water, Fire}` sets only that effect's per-draw uniforms (`Renderer::wind/water/fire`). The bounds of animated effects are padded for culling.
- `LightingController` is pure data: `light` plus at most `MAX_POINT_LIGHTS` point lights. `FogController` is data too.
- `util::model_matrix(...)` overloads are in `engine/util/Transform.hpp`. Note that `model_matrix(t, r, s)` computes **R*T*S** (rotation is applied before translation); scene coordinates are authored against that.
- `util::AABB` and `util::Frustum` are in `engine/util/Geometry.hpp`.

### Shader conventions (`engine/graphics/ShaderInterface.hpp` <-> `resources/shaders/include/frame.glsl`)
- Scene shaders do `#include "frame.glsl"` to get the std140 `FrameUniforms` block: `projection`, `view`, `viewPos`, `time`, `light*`, `pointLights[]`, `fog*`, cascade data. **Keep the C++ struct and the GLSL block in sync**; `static_assert`s check the C++ offsets.
- `#include "shadows.glsl"` gives `calculateShadow(fragPos, normal)` (cascaded, hardware PCF, normal offset). `#include "fog.glsl"` gives `applyFog(color, fragPos)`.
- Includes resolve from `resources/shaders/include/` and are applied per stage, each file at most once. Only `*.glsl` files directly in `resources/shaders/` are loaded as shaders.
- Texture units are fixed: diffuse 0, specular 1, normal 2, height 3, and `shadowMap` 7 (a `sampler2DArrayShadow`). Samplers and the uniform-block binding are assigned once at link time (`Shader::apply_conventions`).
- `Shader` caches all uniform locations at link time. The setters take `std::string_view` and never call `glGetUniformLocation`. **`set_*` affects the currently bound program**, so call `use()` first.
- Instanced shaders read `mat4 instanceMatrix` from attribute location 5 (5-8).

### Shadows (`ShadowController`)
- `CASCADE_COUNT` = 4 cascades in a `MAP_SIZE`^2 depth texture array. `CASCADE_RADII` = {8, 24, 64, 160} are square areas centered on the camera and snapped to texels.
- A cascade is re-rendered only when the light direction changes, the camera drifts more than 25% of the cascade radius, or `invalidate()` is called. **Call `invalidate()` if shadow casters ever move.** The depth pass doesn't animate wind, so the cached shadows stay exact.

### Resources (`engine/resources`)
`ResourcesController` loads everything at init, relative to the CWD:
- **Shaders:** every `*.glsl` in `resources/shaders/`. The name is the file stem. A single file holds several stages separated by `//#shader vertex`, `//#shader fragment`, and optionally `//#shader geometry`. Linking is checked.
- **Models:** only the ones listed in `config.json` → `resources.models.<name>.path` (relative to `resources/models/`), with optional `"flip_uvs": true`. They're loaded through Assimp; embedded (glb) textures are supported. `Model::bounds()` is the local AABB. Assimp node transforms are *not* applied to meshes.
- **Skyboxes:** subfolders of `resources/skyboxes/` (right/left/top/bottom/front/back images), loaded **on first use** (`skybox(name)`), with the 6 faces decoded in parallel. `MainController` preloads the active day and night skyboxes. **Textures:** `resources/textures/`.
- **Texture loading:** during `ResourcesController::initialize`, `texture()`/`texture_from_memory()` only create the GL texture object and queue the decode. `TextureLoader::load_all()` then decodes on all cores (largest files first) and uploads on the main thread. Outside initialize, loads are synchronous. Decoding goes through `resources::load_image*` (stb_image v2.29, thread-safe per-thread flip). stb is always compiled with `-O2`, even in Debug.
- Only `DIFFUSE`, `BASE_COLOR` and `NORMALS` material textures are loaded (see `process_materials`), because shaders sample nothing else. Add types there when a shader needs them (e.g. PBR).
- **Sounds:** loaded by `SoundController` (miniaudio), e.g. `sound("ocean_wind.wav")`.

To add a model: put the assets in `app/resources/models/<dir>/`, add an entry to `app/config.json`, then add it to the `SCENE` table in `MainController.cpp`.

### Other engine controllers
- `PlatformController`: GLFW window, input (`key(KEY_X).state() == Key::State::JustPressed`, `is_down()`), `dt()`, `get_time()`.
- `FPSCameraController`: WASD/arrows to move, Space/C for up/down, Shift to sprint, Q/E to rotate, P to toggle the cursor.
- `TimeController`: in-game clock. Starts at 12:00, `tickrate` is 0.2 in-game hours per real second, night is 23:00–07:00.
- `FogController`, `GreyscaleController`: parameter holders, read by the shaders and post-processing. Like `RainController`, they're declared in the global namespace, not in `engine::`.
- `RainController`: real 3D rain.
  - Up to 40k camera-facing streaks, generated in `rain.glsl` from `gl_InstanceID` plus time (no vertex data, empty VAO). They're world-stable and wrap around a 40×24×40 box around the camera.
  - It's drawn in its own `draw()` after `MainController` (after the skybox) and before the GUI. Drops are depth-tested, don't write depth, and don't write to the bloom attachment (`glColorMaski`). They have their own "Rain" profiler section.
  - Intensity eases in and out over `FADE_SECONDS`. `wetness` follows it (wets in ~15 s, dries in ~45 s).
  - Both values go to all shaders through `FrameUniforms` (`wetness`, `rainIntensity`). `include/weather.glsl` has `surfaceWetness`/`wetAlbedo`/`wetSpecular`, used by the lit shaders, and `rainRipples`, used by water. Wind slants the drops.

## App (`app/`)
- `MyApp.cpp` registers the app controllers and the optional engine controllers and sets their order.
- `MainController`:
  - The scene is the `static constexpr SCENE` table in `MainController.cpp`. Each entry has a model, a shader, transforms, a `DrawMethod`, an `Effect`, and `casts_shadow`. `create_scene()` turns it into `DrawBatch`es once. `draw()` draws them in table order, and `render_depth_scene()` draws the shadow casters from the same table. The fire is separate because it's only drawn at night.
  - Handles the day/night transition: it's triggered by the clock or the `N` key, and interpolates exposure and fog over `DAY_CHANGE_DELAY` = 5s. It swaps the lighting preset, and adds the campfire point light and fire effect at night.
  - Handles hotkeys: `F` fog, `V` wind, `R` rain, `M` sound.
  - Turns on the underwater tint when camera y < 7.
  - Implements the test-model placement tool. The preview is drawn only while the Debug tab is open (`model_placement_active`). Placed models are logged as `TransformData` literals on exit so they can be pasted into coordinate files.
- `GUIController`: ImGui menu opened with `ESC`. Has a main page and Settings with Graphics/Environment/Sound/Time/Debug tabs, plus resolution/fullscreen and quit.
- Static placements are in `app/include/coordinates/*.include`: comma-separated `{{t},{r},{s}}` literals `#include`d into `constexpr std::array<TransformData, N>` in `MainController.cpp`. **Update `N` when you add or remove entries.**
- Shadow casting only happens during the day (`render_depth_scene` returns early at night).

## Gotchas
- `app/CMakeLists.txt` and `engine/CMakeLists.txt` use `file(GLOB ...)`. Re-run CMake after adding source files.
- `imgui.ini` files are runtime-generated (`app/imgui.ini` is tracked and gets modified whenever the app runs).
- `build/`, `build-*/`, `cmake-build-*/`, and `venv/` are local build artifacts and are gitignored. They contain machine-specific paths (e.g. GLFW's generated `cmake_uninstall.cmake`); never commit them.
- **Measure performance in Release** (`-DCMAKE_BUILD_TYPE=Release`). In Debug, `CHECKED_GL_CALL` calls `glGetError` after every call, but the frame is GPU-bound so Debug is usually close.
- `engine/CMakeLists.txt` forces `SPDLOG_USE_STD_FORMAT=ON`, because spdlog's bundled fmt doesn't compile with recent Clang. Log preformatted `std::format` strings (`spdlog::info(std::format(...))`), not fmt-style arguments.
- `check.py`: namespace-scope constants must be `static constexpr` to be allowed UPPER_CASE names. It crashes on `.DS_Store` files in `app/include`.
- macOS caps OpenGL at 4.1: no compute shaders, SSBOs, `glDrawElementsInstancedBaseInstance` or `layout(binding=)`.
