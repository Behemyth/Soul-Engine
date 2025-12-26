// PBR Shader for Soul Engine - Bindless Version
// Implements Cook-Torrance BRDF with GGX/Schlick approximations
// Uses GPU pointers for all data access (No Graphics API pattern)

// ============================================================================
// Bindless Data Structures (GPU Buffer Layouts)
// ============================================================================

// Vertex data stored in GPU buffer - matches PBRVertex layout (48 bytes)
struct PBRVertex
{
    float3 position;   // 12 bytes
    float3 normal;     // 12 bytes
    float4 tangent;    // 16 bytes (w = handedness)
    float2 texCoord;   // 8 bytes
};

// Per-draw instance data
struct InstanceData
{
    float4x4 mvp;           // Model-View-Projection
    float4x4 model;         // Model matrix for world-space calculations
    float4x4 normalMatrix;  // Inverse transpose of model for correct normal transform
};

// Material data stored in GPU buffer
struct MaterialData
{
    float4 baseColor;      // RGB + alpha
    float metallic;
    float roughness;
    float ao;
    float _padding0;
    float4 emissive;       // RGB + intensity
};

// Light structure
struct Light
{
    float3 position;
    float range;
    float3 direction;
    float spotAngle;
    float3 color;
    float intensity;
    uint type;          // 0 = directional, 1 = point, 2 = spot
    uint castShadows;
    float2 _padding;
};

// Scene lighting data
struct SceneLighting
{
    float3 cameraPosition;
    uint lightCount;
    float3 ambientColor;
    float ambientIntensity;
    Light lights[4];
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
// Pixel Shader Data (passed via GPU pointer)
// ============================================================================
struct PixelShaderData
{
    MaterialData* material;      // GPU pointer to MaterialData
    SceneLighting* scene;        // GPU pointer to SceneLighting
    uint baseColorTexture;       // Texture heap index (0xFFFFFFFF = none)
    uint normalMapTexture;       // Texture heap index
    uint metallicRoughnessTexture;
    uint aoTexture;
    uint emissiveTexture;
    uint samplerIndex;           // Sampler heap index
    uint2 _padding;
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
    float3 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float3 worldTangent : WORLD_TANGENT;
    float3 worldBitangent : WORLD_BITANGENT;
    float2 texCoord : TEXCOORD0;
};

// Constants
static const float PI = 3.14159265359;
static const float EPSILON = 0.0001;
static const uint TEXTURE_INDEX_NONE = 0xFFFFFFFF;

// ============================================================================
// Vertex Shader
// ============================================================================

[shader("vertex")]
VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;
    
    // Access vertex/instance data through GPU pointers - load to local copy
    VertexShaderData vsData = *root.vertexData;
    
    // Read vertex from buffer (indexed or direct)
    uint actualVertexId = vertexId;
    if (vsData.indexBuffer != nullptr && vsData.indexCount > 0) {
        actualVertexId = vsData.indexBuffer[vertexId];
    }
    
    // Read vertex data from GPU buffer
    PBRVertex vertex = vsData.vertexBuffer[actualVertexId];
    
    // Get instance transforms - load to local copy
    InstanceData instance = *vsData.instanceData;
    
    // Transform position
    float4 worldPos = mul(instance.model, float4(vertex.position, 1.0));
    output.clipPosition = mul(instance.mvp, float4(vertex.position, 1.0));
    output.worldPosition = worldPos.xyz;
    
    // Transform normal using normal matrix (inverse transpose of model)
    float3 worldNormal = mul((float3x3)instance.normalMatrix, vertex.normal);
    output.worldNormal = normalize(worldNormal);
    
    // Transform tangent
    float3 worldTangent = mul((float3x3)instance.model, vertex.tangent.xyz);
    output.worldTangent = normalize(worldTangent);
    
    // Compute bitangent (tangent.w is handedness)
    output.worldBitangent = cross(output.worldNormal, output.worldTangent) * vertex.tangent.w;
    
    // Pass through texture coordinates
    output.texCoord = vertex.texCoord;
    
    return output;
}

// ============================================================================
// PBR Helper Functions
// ============================================================================

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / max(denom, EPSILON);
}

// Geometry function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / max(denom, EPSILON);
}

// Geometry function (Smith's method)
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel (Schlick approximation)
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// Calculate light attenuation
float CalculateAttenuation(float distance, float range)
{
    float att = saturate(1.0 - (distance / range));
    return att * att;
}

// ============================================================================
// Fragment Shader
// ============================================================================

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    // Access shader data through GPU pointers - load into local copies
    PixelShaderData psData = *root.pixelData;
    MaterialData material = *psData.material;
    SceneLighting scene = *psData.scene;
    
    // Get material properties
    float3 albedo = material.baseColor.rgb;
    float metallic = material.metallic;
    float roughness = max(material.roughness, 0.04);  // Clamp to avoid artifacts
    float ao = material.ao;
    
    // Texture indices from pixel shader data
    uint baseColorTex = psData.baseColorTexture;
    uint normalTex = psData.normalMapTexture;
    uint mrTex = psData.metallicRoughnessTexture;
    uint aoTex = psData.aoTexture;
    uint emissiveTex = psData.emissiveTexture;
    uint samplerIdx = psData.samplerIndex;
    
    // Sample textures if available
    if (baseColorTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float4 texColor = textureHeap[baseColorTex].Sample(
            samplerHeap[samplerIdx], input.texCoord);
        albedo *= texColor.rgb;
    }
    
    // Normal mapping
    float3 N = normalize(input.worldNormal);
    if (normalTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float3 tangentNormal = textureHeap[normalTex].Sample(
            samplerHeap[samplerIdx], input.texCoord).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);
        float3x3 TBN = float3x3(T, B, N);
        N = normalize(mul(tangentNormal, TBN));
    }
    
    // Metallic/roughness texture
    if (mrTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float4 mr = textureHeap[mrTex].Sample(
            samplerHeap[samplerIdx], input.texCoord);
        roughness *= mr.g;
        metallic *= mr.b;
    }
    
    // AO texture
    if (aoTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        ao *= textureHeap[aoTex].Sample(
            samplerHeap[samplerIdx], input.texCoord).r;
    }
    
    // View direction
    float3 V = normalize(scene.cameraPosition - input.worldPosition);
    
    // Fresnel reflectance at normal incidence (F0)
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    // Accumulate lighting
    float3 Lo = float3(0, 0, 0);
    
    // Process each light
    for (uint i = 0; i < min(scene.lightCount, 4); i++) {
        Light light = scene.lights[i];
        
        float3 L;
        float attenuation = 1.0;
        
        if (light.type == 0) {
            // Directional light
            L = normalize(-light.direction);
        } else {
            // Point/spot light
            float3 lightVec = light.position - input.worldPosition;
            float distance = length(lightVec);
            L = lightVec / distance;
            attenuation = CalculateAttenuation(distance, light.range);
            
            if (light.type == 2) {
                // Spot light - add cone attenuation
                float theta = dot(L, normalize(-light.direction));
                float epsilon = light.spotAngle * 0.1;  // Soft edge
                attenuation *= saturate((theta - cos(light.spotAngle)) / epsilon);
            }
        }
        
        float3 H = normalize(V + L);
        float3 radiance = light.color * light.intensity * attenuation;
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);
        
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
        float3 specular = numerator / denominator;
        
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // Ambient lighting
    float3 ambient = scene.ambientColor * scene.ambientIntensity * albedo * ao;
    
    // Emissive
    float3 emissive = material.emissive.rgb * material.emissive.a;
    if (emissiveTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        emissive *= textureHeap[emissiveTex].Sample(
            samplerHeap[samplerIdx], input.texCoord).rgb;
    }
    
    float3 color = ambient + Lo + emissive;
    
    // HDR tonemapping (Reinhard)
    color = color / (color + 1.0);
    
    // Gamma correction
    color = pow(color, 1.0 / 2.2);
    
    return float4(color, material.baseColor.a);
}
