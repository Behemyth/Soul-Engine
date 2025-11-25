# Shader Compilation

Soul Engine uses [Slang](https://shader-slang.com/) for shader authoring with automatic SPIR-V compilation at build time.

## Writing Shaders

Place `.slang` files in `resources/shaders/`. Annotate entry points with shader stage attributes:

```slang
[shader("vertex")]
float4 vertexMain(uint vertexId : SV_VertexID) : SV_Position
{
    // ...
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    // ...
}
```

## Output Naming

CMake automatically discovers shaders and compiles each stage to SPIR-V:

| Source | Attribute | Output |
|--------|-----------|--------|
| `triangle.slang` | `[shader("vertex")]` | `triangle.vertex.spv` |
| `triangle.slang` | `[shader("fragment")]` | `triangle.fragment.spv` |

## Adding New Shaders

1. Create a `.slang` file in `resources/shaders/`
2. Add entry points with `[shader("stage")]` attributes
3. Rebuild — CMake auto-discovers and compiles them

No CMake modifications required.

## Entry Points

Slang compiles each entry point to a separate SPIR-V module with `main` as the entry point name.
