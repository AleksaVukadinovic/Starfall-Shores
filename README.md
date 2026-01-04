# About

Starfall Shores is a custom-built graphics engine created for a university course on Computer Graphics. This project showcases low-level rendering capabilities using modern OpenGL and GLSL. The engine emphasizes both performance and visual fidelity, making it a solid foundation for real-time 3D rendering experiments and interactive visuals.

Key goals included writing everything from scratch, gaining deeper insight into graphics pipelines, and implementing efficient rendering strategies commonly found in game engines.

# Setup

## Linux

### Debian based (Ubuntu, Debian...)

**To setup the necessary libraries, run:**  
`./setup.sh`

**To generate docs, run:**  
`doxygen Doxyfile`   
Open the documentation file in your browser: `docs/html/index.html`

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

## Project structure

Here is how the `Engine` is structured. You only need to include `<engine/core/Engine.hpp>` in your part of the project
and all the header files will be available.

```bash
├── core
│   ├── App.hpp
│   ├── Controller.hpp
│   └── Engine.hpp
├── graphics
│   ├── Camera.hpp
│   ├── GraphicsController.hpp
│   └── OpenGL.hpp
├── platform
│   ├── Input.hpp
│   ├── PlatformController.hpp
│   ├── PlatformEventObserver.hpp
│   └── Window.hpp
├── resources
│   ├── Mesh.hpp
│   ├── Model.hpp
│   ├── ResourcesController.hpp
│   ├── ShaderCompiler.hpp
│   ├── Shader.hpp
│   ├── Skybox.hpp
│   └── Texture.hpp
└── util
    ├── ArgParser.hpp
    ├── Configuration.hpp
    ├── Errors.hpp
    └── Utils.hpp
p
```
