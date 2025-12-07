# Shader Type Safety

Soul Engine provides comprehensive type safety between C++ and Slang shaders through reflection-based code generation and runtime validation.

## Overview

The shader reflection system ensures:
- **Struct layout compatibility** - C++ structs match shader UBO/push constant layouts
- **Binding correctness** - Descriptor set layouts match shader requirements
- **Vertex format matching** - Vertex input layouts match shader expectations
- **Compile-time validation** - Static assertions catch mismatches early

## Architecture

```
┌─────────────────────┐
│   .slang shaders    │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐     ┌─────────────────────┐
│  slang_to_cpp.py    │────▶│  *_reflection.hpp   │  (build-time)
└─────────────────────┘     └──────────┬──────────┘
                                       │
           ┌───────────────────────────┘
           ▼
┌─────────────────────┐     ┌─────────────────────┐
│  SlangReflection    │────▶│  Runtime validation │  (runtime)
└─────────────────────┘     └─────────────────────┘
```

## Build-Time Code Generation

### Using CMake

The `build_slang_shaders` function compiles shaders and generates reflection headers:

```cmake
include(shader_compilation)

build_slang_shaders(
    TARGET game_shaders
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/resources/shaders"
    SPV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders"
    REFLECTION_OUTPUT "${CMAKE_BINARY_DIR}/generated/shader_layouts.hpp"
    NAMESPACE "game::shaders"
)

# Add generated header to include path
target_include_directories(my_target PRIVATE "${CMAKE_BINARY_DIR}/generated")
```

### Generated Code

For a shader like `pbr.slang`:

```slang
struct MaterialData
{
    float4 baseColor;
    float metallic;
    float roughness;
    float ao;
    float _padding0;
    float4 emissive;
};

[[vk::binding(0, 0)]]
ConstantBuffer<MaterialData> material;

[[vk::push_constant]]
PushConstants pushConstants;
```

The generator produces:

```cpp
namespace game::shaders
{

struct PbrShaderLayout
{
    // Descriptor Bindings
    static constexpr std::uint32_t MATERIAL_SET = 0;
    static constexpr std::uint32_t MATERIAL_BINDING = 0;
    
    // Push Constants
    static constexpr std::uint32_t PUSH_CONSTANT_SIZE = 128;
    
    // Binding info for descriptor set layout creation
    struct BindingInfo {
        std::uint32_t set;
        std::uint32_t binding;
        const char* name;
        const char* descriptorType;
    };
    
    static constexpr std::array<BindingInfo, 2> BINDINGS = {{
        { 0, 0, "material", "eUniformBuffer" },
        { 0, 1, "scene", "eUniformBuffer" },
    }};
};

} // namespace game::shaders
```

## Runtime Reflection API

### Reflecting Shaders

```cpp
import synodic.soul.transput;

using namespace synodic::soul::shader;

// Reflect from file
auto result = SlangReflection::ReflectFromFile("shaders/pbr.slang");
if (result)
{
    const ShaderReflectionData& data = *result;
    
    // Enumerate bindings
    for (const auto& binding : data.bindings)
    {
        std::println("Binding: {} at set={}, binding={}", 
            binding.name, binding.set, binding.binding);
    }
    
    // Check push constant size
    std::println("Push constant size: {}", data.TotalPushConstantSize());
}
```

### Validating C++ Structs

```cpp
// Validate that C++ struct matches shader layout
auto validation = LayoutValidator::ValidateStruct<MaterialDataGPU>(
    shaderStruct,
    {
        {"baseColor", offsetof(MaterialDataGPU, baseColor)},
        {"metallic", offsetof(MaterialDataGPU, metallic)},
        {"roughness", offsetof(MaterialDataGPU, roughness)},
        {"ao", offsetof(MaterialDataGPU, ao)},
        {"emissive", offsetof(MaterialDataGPU, emissive)},
    }
);

if (!validation)
{
    for (const auto& error : validation.errors)
    {
        std::println("Layout error: {}", error.message);
    }
}
```

## Descriptor Set Layout Generation

### Automatic Layout Creation

Use reflection data to create Vulkan descriptor set layouts:

```cpp
#include "shader_layouts.hpp"

// Create layout from reflection constants
std::vector<DescriptorBinding> bindings;
for (const auto& info : PbrShaderLayout::BINDINGS)
{
    if (info.set == 0)  // Set 0 bindings
    {
        DescriptorBinding binding;
        binding.binding = info.binding;
        binding.type = StringToDescriptorType(info.descriptorType);
        binding.stages = vk::ShaderStageFlagBits::eAllGraphics;
        bindings.push_back(binding);
    }
}

VulkanDescriptorSetLayout layout(device, bindings);
```

### Manual Override

For cases where generated layouts need modification:

```cpp
// Start with generated bindings, then customize
auto bindings = PbrShaderLayout::GetBindings();

// Add additional binding for shadow maps
bindings.push_back(DescriptorBinding::CombinedImageSampler(
    10, vk::ShaderStageFlagBits::eFragment));

VulkanDescriptorSetLayout layout(device, bindings);
```

## Vertex Input Detection

### Automatic Vertex Layout

The reflection system extracts vertex attributes from shader entry points:

```cpp
// Get vertex layout from reflection
const auto& entryPoint = reflectionData.entryPoints[0];

std::vector<vk::VertexInputAttributeDescription> attrs;
for (const auto& input : entryPoint.inputs)
{
    vk::VertexInputAttributeDescription attr;
    attr.location = input.location;
    attr.binding = 0;
    attr.format = FormatFromReflection(input.scalarType, input.vectorSize);
    attr.offset = input.offset;
    attrs.push_back(attr);
}
```

### Validation Against CPU Vertex Struct

```cpp
// Ensure C++ vertex struct matches shader expectations
static_assert(sizeof(PBRVertex) == PbrShaderLayout::VERTEX_STRIDE,
    "Vertex stride mismatch between C++ and shader");

static_assert(offsetof(PBRVertex, position) == PbrShaderLayout::POSITION_OFFSET,
    "Position offset mismatch");
static_assert(offsetof(PBRVertex, normal) == PbrShaderLayout::NORMAL_OFFSET,
    "Normal offset mismatch");
```

## Push Constant Validation

### Size Validation

```cpp
// Compile-time validation
static_assert(sizeof(PushConstantData) == PbrShaderLayout::PUSH_CONSTANT_SIZE,
    "Push constant size mismatch with shader");

// Runtime validation
auto validation = LayoutValidator::ValidatePushConstantSize(
    sizeof(PushConstantData), reflectionData);
assert(validation);
```

### Creating Pipeline Layout

```cpp
PipelineLayoutConfig config;
config.pushConstantRanges.push_back({
    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
    0,
    PbrShaderLayout::PUSH_CONSTANT_SIZE
});
config.descriptorSetLayouts.push_back(materialLayout.Handle());

VulkanPipelineLayout pipelineLayout(device, config);
```

## Best Practices

### 1. Single Source of Truth

Keep shader definitions as the authoritative source:

```slang
// pbr.slang - defines the contract
struct MaterialData { ... };
```

```cpp
// Use generated types, don't duplicate
using MaterialGPU = shader_layouts::MaterialDataGPU;
```

### 2. Validation in Debug Builds

```cpp
#ifndef NDEBUG
    auto reflection = SlangReflection::ReflectFromFile(shaderPath);
    auto validation = LayoutValidator::ValidatePushConstantSize(
        sizeof(PushConstantData), *reflection);
    assert(validation && "Push constant layout mismatch!");
#endif
```

### 3. Regenerate on Shader Changes

Add shader files as CMake dependencies:

```cmake
# Automatically regenerate when shaders change
add_custom_command(
    OUTPUT "${GENERATED_HEADER}"
    DEPENDS ${SHADER_FILES}
    COMMAND ${Python3_EXECUTABLE} ${GENERATOR_SCRIPT} ...
)
```

### 4. Use Static Assertions

```cpp
// Catch layout changes at compile time
static_assert(alignof(MaterialDataGPU) == 16, 
    "Material struct must be 16-byte aligned for std140");
static_assert(sizeof(MaterialDataGPU) == 48,
    "Material struct size changed - update shader");
```

## Troubleshooting

### Common Issues

**Size Mismatch**
```
error: Struct size mismatch: C++ = 44, shader = 48
```
*Cause*: Missing padding in C++ struct. Add explicit padding fields.

**Offset Mismatch**
```
error: Field 'emissive' offset mismatch: C++ = 32, shader = 36
```
*Cause*: std140 layout rules differ from C++ default. Check alignment.

**Binding Not Found**
```
warning: Binding 'shadowMap' at set=0, binding=5 not in generated layout
```
*Cause*: Shader was modified but reflection header not regenerated.

### Debugging

Enable verbose reflection output:

```cpp
auto data = SlangReflection::ReflectFromFile(path);
for (const auto& s : data->structs)
{
    std::println("Struct: {} (size={}, align={})", s.name, s.size, s.alignment);
    for (const auto& f : s.fields)
    {
        std::println("  {} : {} @ offset {}", f.name, f.typeName, f.offset);
    }
}
```

## API Reference

### ShaderReflectionData

```cpp
struct ShaderReflectionData
{
    std::string moduleName;
    std::vector<ReflectedEntryPoint> entryPoints;
    std::vector<ReflectedBinding> bindings;
    std::vector<ReflectedPushConstant> pushConstants;
    std::vector<ReflectedStruct> structs;
    
    // Convenience methods
    std::optional<ReflectedBinding> FindBinding(uint32_t set, uint32_t binding);
    std::optional<ReflectedStruct> FindStruct(std::string_view name);
    std::size_t TotalPushConstantSize();
};
```

### LayoutValidator

```cpp
class LayoutValidator
{
    template<typename T>
    static LayoutValidationResult ValidateStruct(
        const ReflectedStruct& shaderStruct,
        std::span<const std::pair<std::string, std::size_t>> fieldOffsets);
    
    static LayoutValidationResult ValidatePushConstantSize(
        std::size_t cppSize,
        const ShaderReflectionData& reflection);
};
```

### SlangReflection

```cpp
class SlangReflection
{
    static ShaderResult<ShaderReflectionData> ReflectFromFile(
        const std::filesystem::path& path,
        std::span<const std::string> entryPoints = {});
    
    static ShaderResult<ShaderReflectionData> ReflectFromSource(
        std::string_view source,
        std::string_view sourceName,
        std::span<const std::string> entryPoints = {});
};
