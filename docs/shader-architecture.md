# Shader Module Architecture

## Directory Structure

```
src/transput/resource/shader/
├── shader.ixx              # Public module interface
├── shader_error.ixx        # Error types (backend-agnostic)
├── shader_stage.ixx        # Stage enums, compile options
├── shader_types.ixx        # Reflection data types (backend-agnostic)
├── shader_compiler.ixx     # IShaderCompiler abstract interface
└── backend/
    └── slang/
        └── slang_compiler.ixx  # Slang implementation
```

## Key Design Principles

1. **Backend Isolation**: The Vulkan renderer only depends on `synodic.soul.transput:shader`, never on Slang directly
2. **Pluggable Backends**: New compiler backends (DXC, glslc, etc.) can be added without modifying the renderer
3. **Runtime Reflection**: Shader introspection happens at load time using the backend's reflection API

## Component Responsibilities

| Component | Location | Purpose |
|-----------|----------|---------|
| `IShaderCompiler` | `shader_compiler.ixx` | Abstract interface for any compiler backend |
| `SlangShaderCompiler` | `backend/slang/` | Slang-specific implementation |
| `ShaderReflectionData` | `shader_types.ixx` | Backend-agnostic reflection data structures |
| `vulkan_shader_reflection_utils` | Vulkan backend | Converts generic shader types to Vulkan types |

## Adding a New Backend

1. Create directory: `src/transput/resource/shader/backend/<name>/`
2. Create `<name>_compiler.ixx` implementing `IShaderCompiler`
3. Add `CMakeLists.txt` to new directory
4. Include in `backend/CMakeLists.txt`

Example skeleton:
```cpp
export module synodic.soul.transput:<name>_compiler;

import :shader_compiler;

export namespace synodic::soul::shader
{
    class MyCompiler final : public IShaderCompiler
    {
        // Implement all virtual methods
    };
    
    [[nodiscard]] ShaderResult<ShaderCompiler> CreateMyCompiler();
}
```

## C++26 Reflection Migration

When C++26 arrives, the reflection types in `shader_types.ixx` will map naturally to compile-time reflection:

| Current | C++26 |
|---------|-------|
| `ReflectedField` populated at runtime | `std::meta::members_of(^T)` at compile time |
| `LayoutValidator::ValidateStruct()` | `static_assert(...)` |
| Runtime `ShaderReflectionData` | `consteval` reflection |

The current architecture is designed for this transition:
- Reflection data is plain POD types
- Validation is separate from data structures  
- Backend-specific code is isolated
