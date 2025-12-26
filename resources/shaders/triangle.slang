// Simple Triangle Shader for Soul Engine - Bindless Version
// Draws a colored triangle using procedural vertices
// Demonstrates minimal bindless shader with no external data

// ============================================================================
// Root Constants (Push Constants) - Bindless Pattern
// ============================================================================
struct RootConstants
{
    uint64_t vertexDataPtr;      // GPU pointer to optional per-draw data
    uint64_t pixelDataPtr;       // GPU pointer to optional per-draw data
};

[[vk::push_constant]]
RootConstants root;

// ============================================================================
// Interpolated Data
// ============================================================================
struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

// ============================================================================
// Hardcoded Procedural Geometry
// ============================================================================

// Hardcoded triangle positions
static const float2 positions[3] = {
    float2(0.0, -0.5),   // Top
    float2(0.5, 0.5),    // Bottom right
    float2(-0.5, 0.5)    // Bottom left
};

// Hardcoded vertex colors
static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0),  // Red
    float3(0.0, 1.0, 0.0),  // Green
    float3(0.0, 0.0, 1.0)   // Blue
};

// ============================================================================
// Vertex Shader
// ============================================================================

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.color = colors[vertexId];
    return output;
}

// ============================================================================
// Fragment Shader
// ============================================================================

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
