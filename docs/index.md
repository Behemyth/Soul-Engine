# Soul Engine Documentation

Soul Engine is a modern C++23 game engine with a focus on GPU-driven rendering using Vulkan.

## Core Features

- **GPU-Driven Rendering** — Mesh shader pipeline with meshlet-based geometry
- **Modern C++** — C++23 modules throughout the codebase
- **Frame Graph** — Automatic resource management and synchronization
- **Bindless Design** — Descriptor indexing for textures and buffers
- **Slang Shaders** — Modern shader language with hot-reload support

## Documentation

### Rendering

- [Mesh Shaders](mesh-shaders.md) — GPU-driven meshlet rendering with `VK_EXT_mesh_shader`
- [Render Graph](render-graph.md) — Frame graph system for pass management

### Shaders

- [Shader Architecture](shader-architecture.md) — Compiler backend design
- [Shader Compilation](shader-compilation.md) — Build-time and runtime compilation
- [Shader Type Safety](shader-type-safety.md) — Type validation between CPU and GPU

### Resources

- [Asset Loading](asset-loading.md) — glTF and asset pipeline

## Quick Start

```cpp
#include <synodic.soul.engine>

int main()
{
    auto& soul = synodic::soul::GetSoul();
    
    // Initialize engine
    soul.Initialize({
        .appName = "My Application",
        .windowWidth = 1920,
        .windowHeight = 1080
    });
    
    // Main loop
    while (soul.IsRunning())
    {
        soul.Update();
        soul.Render();
    }
    
    return 0;
}
```

## Requirements

- **Vulkan 1.3+** with mesh shader extension
- **MSVC 19.40+** (Visual Studio 2022 17.10+) for C++23 modules
- **CMake 4.0+**
- **Conan 2.x** for dependency management

## Building

```bash
# Install dependencies
conan install . --build=missing

# Configure
cmake --preset windows-ninja-msvc

# Build
cmake --build --preset windows-ninja-msvc
```
