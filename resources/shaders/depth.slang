// Depth-Only Shader for Soul Engine - Bindless Version
// Used for depth pre-pass to populate depth buffer before main PBR pass
// Uses GPU pointers for all data access (No Graphics API pattern)

// ============================================================================
// Bindless Data Structures
// ============================================================================

// Vertex data stored in GPU buffer - matches PBRVertex layout (48 bytes)
struct PBRVertex
{
    float3 position;   // 12 bytes
    float3 normal;     // 12 bytes (unused in depth pass)
    float4 tangent;    // 16 bytes (unused in depth pass)
    float2 texCoord;   // 8 bytes (unused in depth pass)
};

// Per-draw instance data
struct InstanceData
{
    float4x4 mvp;           // Model-View-Projection
    float4x4 model;         // Unused in depth pass
    float4x4 normalMatrix;  // Unused in depth pass
};

// ============================================================================
// Vertex Shader Data (passed via GPU pointer)
// ============================================================================
struct VertexShaderData
{
    PBRVertex* vertexBuffer;     // GPU pointer to PBRVertex[]
    uint* indexBuffer;           // GPU pointer to uint32[] (null = non-indexed)
    InstanceData* instanceData;  // GPU pointer to InstanceData
    uint vertexCount;
    uint indexCount;
    uint instanceId;
    uint _padding;
};

// ============================================================================
// Root Constants (Push Constants) - Bindless Pattern
// ============================================================================
struct RootConstants
{
    VertexShaderData* vertexData;  // GPU pointer to VertexShaderData
    void* pixelData;               // Unused in depth pass
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root;

// ============================================================================
// Output Structure
// ============================================================================
struct DepthOnlyOutput
{
    float4 position : SV_Position;
};

// ============================================================================
// Vertex Shader
// ============================================================================

[shader("vertex")]
DepthOnlyOutput vertexMain(uint vertexId : SV_VertexID)
{
    // Load vertex shader data from GPU pointer
    VertexShaderData* vsData = root.vertexData;
    
    // Resolve vertex index (indexed or direct)
    uint vertexIndex = vertexId;
    if (vsData->indexBuffer)
    {
        vertexIndex = vsData->indexBuffer[vertexId];
    }
    
    // Load only position (other attributes unused in depth pass)
    float3 position = vsData->vertexBuffer[vertexIndex].position;
    
    // Load MVP matrix
    float4x4 mvp = vsData->instanceData->mvp;
    
    DepthOnlyOutput output;
    output.position = mul(mvp, float4(position, 1.0));
    return output;
}

// ============================================================================
// Fragment Shader
// ============================================================================

// Empty fragment shader - only depth is written
[shader("fragment")]
void fragmentMain(DepthOnlyOutput input)
{
    // Depth-only pass - no color output
    // Depth is automatically written from vertex position
}
