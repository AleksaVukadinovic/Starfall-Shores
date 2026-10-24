# About

Starfall Shores is a custom-built graphics engine created for a university course on Computer Graphics. This project showcases low-level rendering capabilities using modern OpenGL and GLSL. The engine emphasizes both performance and visual fidelity, making it a solid foundation for real-time 3D rendering experiments and interactive visuals.

Key goals included writing everything from scratch, gaining deeper insight into graphics pipelines, and implementing efficient rendering strategies commonly found in game engines.

## Features

- **Scene:** an island camp with tents, a campfire, a lighthouse, forests, flowers, grass, and water
- **Day/night cycle:** driven by an in-game clock, with smooth exposure and fog transitions and a campfire light at night
- **Cascaded shadow maps:** four cascades follow the camera, with soft filtered edges, and are re-rendered only when they go stale
- **HDR rendering with bloom:** mip-chain downsample/upsample bloom, exposure tone mapping, and greyscale and underwater screen effects
- **Endless ocean:** a camera-following Gerstner wave surface with sky reflections, sun glints, and foam, configurable in the settings
- **Rain:** 3D rain streaks with wet surfaces and raindrop ripples on the water
- **Snow:** drifting, fluttering snowflakes and snow that settles on the scene in growing patches, then melts
- **Effects:** wind-animated vegetation, a procedural fire shader, and fog
- **Performance:** frustum culling, instanced rendering with persistent GPU buffers, and one per-frame uniform buffer shared by all shaders
- **Built-in GPU/CPU profiler:** open **Settings → Debug** in the pause menu to see per-pass timings, draw calls, and culling statistics
- **Scene file:** every model placement lives in one JSON file, `app/resources/scene.json`
- **Underwater world:** sandy sea floor, kelp forests, schools of fish (boids), caustics, light shafts, water that absorbs red first, Snell's window when looking up, plankton and bubbles, and muffled sound; swim in walk mode
- **Particles:** GPU particle emitters defined in the scene file: embers and smoke over the campfire, fireflies at night, pollen by day
- **Walk mode:** walk on the terrain with gravity and jumping (Settings → Graphics → Camera)
- **In-app editor:** ImGui settings (graphics, environment, sound, time) and a model placement tool
- **3D models:** loaded with Assimp (glTF, GLB, OBJ, ...)
- **Sound:** played through miniaudio

# Setup

Clone the repository together with its submodules (GLFW and spdlog):

```bash
git clone --recursive <repository-url>
# or, in an existing clone:
git submodule update --init --recursive
```

## Dependencies

The setup script installs the build tools, Assimp, and a Python virtual environment with `libclang` (used by `check.py`):

```bash
./setup.sh
```

It supports **macOS** (through Homebrew) and **Debian-based Linux** (Ubuntu, Debian, ...).

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable is written to `app/APP`. CLion works out of the box: open the project folder and run the `APP` target.

## Run

The app loads `config.json` and `resources/` relative to the working directory, so **run it from the `app/` directory**:

```bash
cd app && ./APP
```

## Controls

| Key | Action |
|---|---|
| `W` `A` `S` `D` / arrow keys | Move |
| `Space` / `C` | Move up / down (in walk mode, `Space` jumps) |
| `Shift` | Sprint (in walk mode: run) |
| `Q` / `E` | Rotate |
| Mouse | Look around |
| `Esc` | Pause menu and settings |
| `N` | Toggle day / night |
| `F` | Toggle fog |
| `R` | Toggle rain |
| `B` | Toggle snow |
| `V` | Toggle wind |
| `M` | Toggle sound |
| `P` | Toggle cursor |

## Code style check

`check.py` verifies the project naming conventions and include rules:

```bash
./venv/bin/python check.py app
./venv/bin/python check.py engine
```

## Documentation

```bash
doxygen Doxyfile
```

Then open `docs/html/index.html` in your browser.

# Engine

## Main loop

```cpp
class App {
public:
int run(int argc, char** argv) {
  try {
    engine_setup(argc, argv);
    app_setup();
    initialize();
    while (loop()) {
        poll_events();
        update();
        draw();
    }
    terminate();
  } catch(...) { ... }
  return on_exit();
}
};
```

* `engine_setup` - here, the engine controllers are setup
* `app_setup` - the function that the user of the `App` overrides and implements a custom setup for the App
* `initialize` - `App` should gather whatever `Resources` it needs and initialize its state.
* `loop` - `App` can check whether it should continue running. If the `loop` method returns `false`,
  the `Main loop` stops, and the `App` terminates.
* `poll_events` - `App` collects information about the events that happened at the `Platform` and collects user input
  for the upcoming frame.
* `update` - `App` updates the world state, processes physics, events, and world logic, and reacts to the user inputs.
* `draw` - `App` uses `OpenGL` and draws the current state of the world.
* `terminate` - `App` terminates its state
* `on_exit` - do a final cleanup, and return an exit code

All of these phases are forwarded to **controllers**: singletons that you register in `app_setup` with
`register_controller<T>()` and access anywhere with `Controller::get<T>()`. Their execution order is defined with
`before()` / `after()` and resolved with a topological sort.

## The App class

Here is the interface of the `engine/core/App.hpp` class:

```cpp
class App {
    public:
        int run(int argc, char **argv);
        virtual ~App() = default;
    private:
        void engine_setup(int argc, char **argv);
        void initialize();
        void poll_events();
        bool loop();
        void update();
        void draw();
        void terminate();
        virtual void app_setup() { // the user extends and implements setup }
        virtual int on_exit() { return 0; }
        virtual void handle_error(const Error &);
    };
```

## Rendering

The `Renderer` draws `DrawBatch`es: a model placed at a fixed set of transforms. A batch is created once, and its model
matrices, bounding boxes, and (for instanced batches) GPU instance buffer are prepared up front:

```cpp
const auto *trees = renderer->create_batch(resources->model("pine_tree"), PINE_TREES, DrawMethod::Instanced);
// every frame:
renderer->draw(trees, resources->shader("flower_shader"));
```

Every frame, the renderer re-renders only the stale shadow cascades, uploads camera, lighting, and fog data into a
single uniform buffer, and frustum-culls each draw.

Shaders are single `.glsl` files with `//#shader vertex` and `//#shader fragment` sections. They can share code with
`#include "file.glsl"` from `resources/shaders/include/`. `frame.glsl` provides the per-frame uniforms,
`shadows.glsl` provides shadow sampling, and `fog.glsl` provides fog.

## Project structure

You only need to include `<engine/core/Engine.hpp>` in your part of the project and all the header files will be available.

```bash
├── core
│   ├── App.hpp                 # main loop
│   ├── Controller.hpp          # controller base class and registration
│   └── Engine.hpp              # includes everything
├── physics
│   └── HeightField.hpp         # ground height queries for walk mode
├── graphics
│   ├── Camera.hpp
│   ├── DrawBatch.hpp           # prepared static placements of a model
│   ├── FPSCameraController.hpp
│   ├── FlockController.hpp     # schools of fish (boids)
│   ├── FogController.hpp
│   ├── GpuProfiler.hpp         # per-section GPU/CPU frame timings
│   ├── GraphicsController.hpp
│   ├── GreyscaleController.hpp
│   ├── LightSource.hpp
│   ├── LightingController.hpp
│   ├── ParticleController.hpp  # stateless GPU particle emitters
│   ├── OpenGL.hpp
│   ├── PostProcessingController.hpp  # HDR, bloom, screen effects
│   ├── RainController.hpp      # 3D rain and wetness
│   ├── Renderer.hpp            # culling, batching, shadow passes
│   ├── ShaderInterface.hpp     # C++ side of the shader uniform layout
│   ├── ShadowController.hpp    # cascaded shadow maps
│   └── WaterController.hpp     # Gerstner wave ocean
├── platform
│   ├── Input.hpp
│   ├── PlatformController.hpp
│   ├── PlatformEventObserver.hpp
│   ├── TimeController.hpp      # in-game clock
│   └── Window.hpp
├── resources
│   ├── InstanceBuffer.hpp
│   ├── Mesh.hpp
│   ├── Model.hpp
│   ├── ResourcesController.hpp
│   ├── ShaderCompiler.hpp
│   ├── Shader.hpp
│   ├── Skybox.hpp
│   ├── Sound.hpp
│   └── Texture.hpp
├── sound
│   └── SoundController.hpp
└── util
    ├── ArgParser.hpp
    ├── Configuration.hpp
    ├── Errors.hpp
    ├── Geometry.hpp            # AABB and frustum
    ├── Transform.hpp
    └── Utils.hpp
```
