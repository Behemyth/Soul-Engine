// UI Shader for Soul Engine - Bindless Version
// Renders 2D UI elements with texture support and per-vertex coloring
// Uses GPU pointers for vertex data (No Graphics API pattern)

// ============================================================================
// Bindless Data Structures
// ============================================================================

// UI vertex stored in GPU buffer - matches UIVertex layout (20 bytes)
struct UIVertex
{
    float2 position;   // Screen-space position (8 bytes)
    float2 uv;         // Texture coordinates (8 bytes)
    uint   color;      // Packed RGBA8 color (4 bytes)
};

// ============================================================================
// Vertex Shader Data (passed via GPU pointer)
// ============================================================================
struct VertexShaderData
{
    UIVertex* vertexBuffer;      // GPU pointer to UIVertex[]
    uint16_t* indexBuffer;       // GPU pointer to uint16[]
    float2 screenSize;           // Viewport dimensions for NDC conversion
    uint vertexCount;
    uint indexCount;
};

// ============================================================================
// Pixel Shader Data (passed via GPU pointer)
// ============================================================================
struct PixelShaderData
{
    uint textureIndex;       // Index into texture heap (0xFFFFFFFF = no texture)
    uint samplerIndex;       // Index into sampler heap
    uint flags;              // Bit flags: 0x1 = has texture, 0x2 = SDF text
    float sdfEdge;           // SDF edge threshold (for text)
    float sdfSmoothing;      // SDF smoothing (for text)
    float _padding;
};

// ============================================================================
// Root Constants (Push Constants) - Bindless Pattern
// ============================================================================
struct RootConstants
{
    VertexShaderData* vertexData;  // GPU pointer to VertexShaderData
    PixelShaderData* pixelData;    // GPU pointer to PixelShaderData
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root;

// ============================================================================
// Texture/Sampler Heaps (Descriptor Indexing)
// ============================================================================
// Note: Sampler at binding 0, Texture at binding 1 due to Vulkan requirement
// that VariableDescriptorCount can only be on the last binding
[[vk::binding(0, 0)]]
SamplerState samplerHeap[];

[[vk::binding(1, 0)]]
Texture2D textureHeap[];

// ============================================================================
// Interpolated Data
// ============================================================================
struct VertexOutput
{
    float4 clipPosition : SV_Position;
    float2 uv           : TEXCOORD0;
    float4 color        : COLOR;
};

// Constants
static const uint TEXTURE_INDEX_NONE = 0xFFFFFFFF;
static const uint UI_FLAG_HAS_TEXTURE = 0x1;
static const uint UI_FLAG_SDF_TEXT = 0x2;

// Unpack RGBA8 packed color to float4
float4 UnpackColor(uint packed)
{
    float4 color;
    color.r = float((packed >> 0) & 0xFF) / 255.0;
    color.g = float((packed >> 8) & 0xFF) / 255.0;
    color.b = float((packed >> 16) & 0xFF) / 255.0;
    color.a = float((packed >> 24) & 0xFF) / 255.0;
    return color;
}

// ============================================================================
// Vertex Shader
// ============================================================================

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    // Load vertex shader data from GPU pointer
    VertexShaderData* vsData = root.vertexData;
    
    // Resolve vertex index (indexed or direct)
    uint vertexIndex = vertexId;
    if (vsData->indexBuffer)
    {
        vertexIndex = vsData->indexBuffer[vertexId];
    }
    
    // Load vertex from GPU buffer
    UIVertex vertex = vsData->vertexBuffer[vertexIndex];
    
    VertexOutput output;
    
    // Convert from screen coordinates to NDC
    // Screen: (0,0) top-left, (width,height) bottom-right
    // NDC: (-1,-1) bottom-left, (1,1) top-right
    float2 ndc;
    ndc.x = (vertex.position.x / vsData->screenSize.x) * 2.0 - 1.0;
    ndc.y = 1.0 - (vertex.position.y / vsData->screenSize.y) * 2.0; // Flip Y
    
    output.clipPosition = float4(ndc, 0.0, 1.0);
    output.uv = vertex.uv;
    output.color = UnpackColor(vertex.color);
    
    return output;
}

// ============================================================================
// Fragment Shader
// ============================================================================

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    // Load pixel shader data from GPU pointer
    PixelShaderData* psData = root.pixelData;
    
    float4 color = input.color;
    
    bool hasTexture = (psData->flags & UI_FLAG_HAS_TEXTURE) != 0;
    bool isSDF = (psData->flags & UI_FLAG_SDF_TEXT) != 0;
    
    if (hasTexture && psData->textureIndex != TEXTURE_INDEX_NONE)
    {
        float4 texColor = textureHeap[psData->textureIndex].Sample(
            samplerHeap[psData->samplerIndex], input.uv);
        
        if (isSDF)
        {
            // SDF text rendering
            float distance = texColor.a;
            float edge = psData->sdfEdge;
            float smoothing = psData->sdfSmoothing;
            
            float alpha = smoothstep(edge - smoothing, edge + smoothing, distance);
            color.a *= alpha;
        }
        else
        {
            // Standard texture blending
            color *= texColor;
        }
    }
    
    return color;
}
