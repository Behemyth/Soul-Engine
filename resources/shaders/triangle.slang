// Simple triangle shader for testing
// Draws a colored triangle using vertex ID

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

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

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;
    output.position = float4(positions[vertexId], 0.0, 1.0);
    output.color = colors[vertexId];
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    return float4(input.color, 1.0);
}
