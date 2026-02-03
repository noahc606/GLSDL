# GLSDL - OpenGL Implementation of SDL2 Renderer

A custom C++ library that provides an OpenGL-based implementation of SDL2's 2D rendering API. This project demonstrates how SDL2's renderer functionality can be replicated using OpenGL.

## Overview

The purpose of GLSDL is to enable seamless integration of SDL2 rendering code within existing OpenGL projects, or to convert existing SDL2 projects to use OpenGL. This allows developers to:

- **Add SDL2-style 2D rendering to existing OpenGL applications**: Easily incorporate UI elements, sprites, and 2D overlays into 3D OpenGL scenes
- **Migrate SDL2 projects to OpenGL**: Gradually transition SDL2 renderer code to OpenGL while maintaining functionality.
- **Mix SDL2 and OpenGL rendering**: Use GLSDL's state management to alternate between SDL2-style rendering and raw OpenGL calls within the same frame
- **Understand rendering pipeline internals**: See how 2D rendering APIs translate to OpenGL operations

The library maintains API compatibility with SDL2's renderer interface, making it a drop-in replacement for SDL2 renderer functions. You can switch between native SDL2 rendering and custom OpenGL rendering via a compile-time flag to compare performance and behavior.

I have since integrated GLSDL into my other projects, with great results:
- NCH-RmlUI-Utils: SDL_Webviews are now rendered from OpenGL, making it easier to implement 2D UIs for 3D OpenGL projects.
- NCH-CPP-Utils: Simple text rendering and MainLoopDriver's performance profiler are now usable in OpenGL contexts.

## Features

- **Drop-in SDL2 Renderer Replacement**: Renderer functions mirror SDL2's renderer API (`GLSDL_RenderCopy`, `GLSDL_CreateTexture`, etc.).
- **Multiple Texture Access Modes**:
  - `SDL_TEXTUREACCESS_STATIC`: For static textures loaded from images
  - `SDL_TEXTUREACCESS_STREAMING`: For dynamic textures with lock/unlock operations
  - `SDL_TEXTUREACCESS_TARGET`: For render-to-texture functionality
- **Full 2D Rendering Support**:
  - Texture rendering with rotation, flipping, and color/alpha modulation
  - Geometry rendering with custom vertices and indices
  - Primitive shapes (rectangles, lines, points)
  - Blend modes (NONE, BLEND, ADD, MOD, MUL)
  - Viewports and clipping rectangles
- **Backend Selection**: Compile-time switch between SDL2 and OpenGL backends. Useful if you want to use pure SDL2 (no OpenGL) with NCH-CPP-Utils and NCH-RmlUi-Utils.

## Demo Application

The included demo (`src/main/Main.cpp`) showcases:
- Loading and rendering PNG images
- Dynamic texture updates (pixel-level modifications)
- Render-to-texture operations
- Animated sprites with cycling blend modes and flip modes
- Viewport and clipping demonstrations
- Geometry rendering with textured quads

## Dependencies

- **SDL2** - Core windowing and event handling
- **SDL2_image** - Image loading, saving, and surface manipulation
- **OpenGL** - Graphics rendering
- **GLEW** - OpenGL extension loading
- **GLM** - OpenGL Mathematics library
- **nch-cpp-utils** - My C++ utility library. Used modules: cpp-utils, math-utils, (in demo only) sdl-utils.

### Integrating nch-cpp-utils

Clone the [nch-cpp-utils](https://github.com/noahc606/nch-cpp-utils) repository and place the following `/include` modules in your project's include path:
- `cpp-utils` - General C++ utilities
- `math-utils` - Mathematics helpers
- `sdl-utils` - SDL2 utility wrappers. Only needed if you are building the example - code within 'include/GLSDL' does not depend on 'sdl-utils'.

```bash
git clone https://github.com/noahc606/nch-cpp-utils.git
```

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev \
                     libglew-dev libglm-dev
```

### Installing Dependencies (macOS)

```bash
brew install sdl2 sdl2_image sdl2_ttf glew glm
```

## Building

### Configure Backend

Edit `CMakeLists.txt` line 2 to choose your backend:

```cmake
set(NCH_GLSDL_OPENGL_BACKEND 0)  # 0 = SDL Backend, 1 = OpenGL Backend
```

### Build with CMake

```bash
mkdir -p build
cd build
cmake ..
make
```

The executable will be generated in the `bin/` directory.

### Run

```bash
cd bin
./GLSDL-SDL_Backend-unix-x86_64       # If using SDL backend
./GLSDL-OpenGL_Backend-unix-x86_64    # If using OpenGL backend
```

**Note**: The demo requires asset files (`test.png`, `clippy.png`, `BackToEarth.ttf`) in the same directory as the executable.

## Setup

```cpp
#include <GLSDL/GLSDL.h>

// Initialize
GLSDL_Init(SDL_INIT_VIDEO);
GLSDL_Window* window = GLSDL_CreateWindow("My Window",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    800, 600, SDL_WINDOW_RESIZABLE);
GLSDL_Renderer* renderer = GLSDL_CreateRenderer(window, -1,
    SDL_RENDERER_ACCELERATED);
```

## Controls (Demo)

- **ESC**: Pause/unpause
- **C**: Toggle clipping demo
- **S**: Screenshot (demo of GLSDL_RenderReadPixels)
- **SPACE**: Advance animations by one tick. Useful while paused.

## Technical Details

### Comparison of Backends

| Feature | SDL Backend | GLSDL Backend |
|---------|-------------|----------------|
| Implementation | Native SDL2 renderer (many different backends like Metal, DirectX, OpengL) | SDL2+OpenGL context, SDL_Renderer implemented using OpenGL functions |
| Performance | Standard SDL2 | Little bit worse than SDL2 or a lot worse, depending on what you are doing |
| Platforms | All SDL2 platforms | SDL2 platform w/ OpenGL 3.3+ required |
| OpenGL 3D integration | Finicky due to SDL2's abstracted backend. No save/restore GL state. | Easy integration

### State Management

When mixing GLSDL (SDL2-style) rendering with raw OpenGL calls, it's crucial to manage OpenGL state properly. GLSDL provides `GLSDL_GL_SaveState()` and `GLSDL_GL_RestoreState()` functions that save and restore necessary parts of the OpenGL context state, allowing seamless integration of both rendering approaches. Also, `assert`s within the necessary functions ensure that you are using this system correctly.

**How it works:**
- `GLSDL_GL_SaveState(renderer)` - Captures the current OpenGL state before GLSDL rendering. Also sets up the OpenGL state for the GLSDL2 functions. In this state you can use the `GLSDL_Render*` functions.
- `GLSDL_GL_RestoreState(renderer)` - Restores the saved state after GLSDL rendering completes. You can now go back to using the `gl*` OpenGL functions.

This allows you to alternate between SDL2-style rendering (GLSDL) and native OpenGL rendering without conflicts or visual artifacts.

**Performance considerations:**
- State save/restore has minimal performance impact when called only a few times per frame
- Best practice: Save/restore only a few times per rendering phase (2D UI, 3D scene, etc.)
- Also safe to use during object initialization without performance concerns

**Example:** Here's how the demo application (`src/main/Main.cpp`) structures its rendering loop:

```cpp
void Main::draw() {
    // Phase 1: Clear screen using GLSDL
    GLSDL_GL_SaveState(glsdlRend);
    GLSDL_SetRenderDrawColor(glsdlRend, r, g, b, 255);
    GLSDL_RenderClear(glsdlRend);
    GLSDL_GL_RestoreState(glsdlRend);

    // Phase 2: 3D rendering with raw OpenGL
    draw3D();  // Uses glEnable(GL_DEPTH_TEST), custom shaders, etc.

    // Phase 3: 2D UI overlay using GLSDL
    GLSDL_GL_SaveState(glsdlRend);
    draw2D();  // Uses GLSDL_RenderCopy, GLSDL_RenderGeometry, etc.
    GLSDL_GL_RestoreState(glsdlRend);
}
```

### Limitations and Unsupported Features

While GLSDL implements most of SDL2's renderer functionality, there are some important limitations:

1. **Pixel Format Restrictions**: SDL pixel formats are converted to the "closest readily availble" OpenGL pixel format. For best compatibility, use **32 bits per pixel** surfaces and textures when working with `STREAMING` textures and `Lock`/`Unlock`/`Update` operations. Other bit depths may not convert correctly.

2. **Texture Access Mode Differences**:
   - `SDL_TEXTUREACCESS_STATIC` textures **do not support** `Lock`/`Unlock`/`Update` operations. For dynamic pixel modifications, you must use `SDL_TEXTUREACCESS_STREAMING` textures
   - See `src/main/Main.cpp` for an example of properly updating streaming textures

3. **Unimplemented Functions**:
   - `SDL_RenderGeometryRaw`.
   - `SDL_GetError`: Functions should return non-positive on errors. Unlike in SDL2, some errors are directly logged when they happen. However, not all errors are logged.
   - Some esoteric or rarely-used SDL2 renderer functions may not be available. When in doubt, check `include/GLSDL/GLSDL_render.h` for the complete API.

4. **OpenGL Version Support**:
   - Internally configured for **OpenGL 3.3** by default
   - Newer OpenGL versions will likely work by modifying `SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)` and `SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)` in `include/GLSDL/GLSDL.cpp`
   - Older versions of OpenGL (pre-3.3) are **not supported**

## Development Status

This library is functionally finished. I don't plan on implementing most of the "Limitations and Unsupported Features" as good workarounds for most of them exist. In time I might implement the functions listed in "Unimplemented Functions" but not right now as I have no need for them. If I discover bugs, I will fix them. Also, there is SDL3 but this library will remain tied to SDL2 for the forseeable future.

## License

GLSDL is licensed under the [MIT License](LICENSE).

Copyright (c) 2026 Noah C. Hebert

All dependencies use permissive open-source licenses that are compatible with MIT. See [THIRD-PARTY-LICENSES](THIRD-PARTY-LICENSES) for detailed information about third-party library licenses.

## Acknowledgments
- SDL2 development team
- OpenGL development team and community
