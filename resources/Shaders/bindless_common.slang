// Bindless Shader Common - Shared Data Structures
// This file defines the GPU data layouts used by all bindless shaders
// These structures must match the C++ side exactly
//
// Slang uses native pointer syntax (T*) which translates to 
// PhysicalStorageBuffer pointers in SPIR-V (VK_KHR_buffer_device_address)

#ifndef BINDLESS_COMMON_SLANG
#define BINDLESS_COMMON_SLANG

// ============================================================================
// Common Vertex Formats
// ============================================================================

// PBR vertex - 48 bytes, matches synodic::soul::PBRVertex
struct PBRVertex
{
    float3 position;   // 12 bytes, offset 0
    float3 normal;     // 12 bytes, offset 12
    float4 tangent;    // 16 bytes, offset 24 (w = handedness)
    float2 texCoord;   // 8 bytes, offset 40
};

// UI vertex - 20 bytes, matches synodic::soul::UIVertex
struct UIVertex
{
    float2 position;   // 8 bytes, offset 0 (screen space)
    float2 uv;         // 8 bytes, offset 8
    uint   color;      // 4 bytes, offset 16 (packed RGBA8)
};

// ============================================================================
// Transform Data
// ============================================================================

// Per-instance transform data - 192 bytes
struct InstanceData
{
    float4x4 mvp;           // 64 bytes, offset 0
    float4x4 model;         // 64 bytes, offset 64
    float4x4 normalMatrix;  // 64 bytes, offset 128 (inverse transpose)
};

// ============================================================================
// Material Data
// ============================================================================

// PBR material - 48 bytes
struct MaterialData
{
    float4 baseColor;      // 16 bytes, offset 0 (RGB + alpha)
    float metallic;        // 4 bytes, offset 16
    float roughness;       // 4 bytes, offset 20
    float ao;              // 4 bytes, offset 24
    float _padding0;       // 4 bytes, offset 28
    float4 emissive;       // 16 bytes, offset 32 (RGB + intensity)
};

// ============================================================================
// Lighting Data
// ============================================================================

// Single light - 64 bytes
struct Light
{
    float3 position;       // 12 bytes, offset 0
    float range;           // 4 bytes, offset 12
    float3 direction;      // 12 bytes, offset 16
    float spotAngle;       // 4 bytes, offset 28
    float3 color;          // 12 bytes, offset 32
    float intensity;       // 4 bytes, offset 44
    uint type;             // 4 bytes, offset 48 (0=directional, 1=point, 2=spot)
    uint castShadows;      // 4 bytes, offset 52
    float2 _padding;       // 8 bytes, offset 56
};

// Scene lighting - 288 bytes (with 4 lights)
struct SceneLighting
{
    float3 cameraPosition;    // 12 bytes, offset 0
    uint lightCount;          // 4 bytes, offset 12
    float3 ambientColor;      // 12 bytes, offset 16
    float ambientIntensity;   // 4 bytes, offset 28
    Light lights[4];          // 256 bytes, offset 32
};

// ============================================================================
// Shader-Specific Data Blocks (using Slang pointer syntax)
// ============================================================================

// PBR Vertex Shader Data
struct PBRVertexShaderData
{
    PBRVertex* vertexBuffer;     // GPU pointer to PBRVertex[]
    uint* indexBuffer;           // GPU pointer to uint32[] (null = non-indexed)
    InstanceData* instanceData;  // GPU pointer to InstanceData
    uint vertexCount;
    uint indexCount;
    uint instanceId;
    uint _padding;
};

// PBR Pixel Shader Data
struct PBRPixelShaderData
{
    MaterialData* material;          // GPU pointer to MaterialData
    SceneLighting* scene;            // GPU pointer to SceneLighting
    uint baseColorTexture;           // Texture heap index (0xFFFFFFFF = none)
    uint normalMapTexture;
    uint metallicRoughnessTexture;
    uint aoTexture;
    uint emissiveTexture;
    uint samplerIndex;
    uint2 _padding;
};

// UI Vertex Shader Data
struct UIVertexShaderData
{
    UIVertex* vertexBuffer;      // GPU pointer to UIVertex[]
    uint16_t* indexBuffer;       // GPU pointer to uint16[]
    float2 screenSize;
    uint vertexCount;
    uint indexCount;
};

// UI Pixel Shader Data
struct UIPixelShaderData
{
    uint textureIndex;       // Texture heap index
    uint samplerIndex;       // Sampler heap index
    uint flags;              // 0x1 = hasTexture, 0x2 = SDF
    float sdfEdge;
    float sdfSmoothing;
    float _padding;
};

// ============================================================================
// Root Constants - Passed via Push Constants to all shaders
// Uses Slang pointers which become PhysicalStorageBuffer in SPIR-V
// ============================================================================

// Generic root constants for any shader type
// Shader-specific code casts these to the appropriate types
struct RootConstants
{
    void* vertexData;    // GPU pointer to shader-specific VertexShaderData
    void* pixelData;     // GPU pointer to shader-specific PixelShaderData
};

// ============================================================================
// Constants
// ============================================================================

static const uint TEXTURE_INDEX_NONE = 0xFFFFFFFF;

// UI flags
static const uint UI_FLAG_HAS_TEXTURE = 0x1;
static const uint UI_FLAG_SDF_TEXT = 0x2;

// Light types
static const uint LIGHT_TYPE_DIRECTIONAL = 0;
static const uint LIGHT_TYPE_POINT = 1;
static const uint LIGHT_TYPE_SPOT = 2;

#endif // BINDLESS_COMMON_SLANG
