// Depth-Only Shader for Soul Engine
// Used for depth pre-pass to populate depth buffer before main PBR pass

// Vertex input - matches PBRVertex layout (48 bytes)
struct VertexInput
{
    float3 position : POSITION;   // location 0
    float3 normal   : NORMAL;     // location 1
    float4 tangent  : TANGENT;    // location 2 (w = handedness)
    float2 texCoord : TEXCOORD0;  // location 3
};

struct DepthOnlyOutput
{
    float4 position : SV_Position;
};

// Push constants (128 bytes) - same as PBR shader
struct PushConstants
{
    float4x4 mvp;    // Model-View-Projection
    float4x4 model;  // Model matrix (unused in depth pass but keeps layout consistent)
};

[[vk::push_constant]]
PushConstants pushConstants;

[shader("vertex")]
DepthOnlyOutput vertexMain(VertexInput input)
{
    DepthOnlyOutput output;
    output.position = mul(pushConstants.mvp, float4(input.position, 1.0));
    return output;
}

// Empty fragment shader - only depth is written
[shader("fragment")]
void fragmentMain(DepthOnlyOutput input)
{
    // Depth-only pass - no color output
    // Depth is automatically written from vertex position
}
