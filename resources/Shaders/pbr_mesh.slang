// PBR Mesh Shader for Soul Engine
// Mesh shader variant of the PBR renderer using meshlet-based geometry
// Uses VK_EXT_mesh_shader for modern GPU-driven rendering

#include "meshlet_common.slang"

// ============================================================================
// Root Constants (Push Constants) - Bindless Pattern
// ============================================================================
struct RootConstants
{
    MeshShaderData* meshData;      // GPU pointer to mesh shader data
    PixelShaderData* pixelData;    // GPU pointer to pixel shader data (same as vertex PBR)
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root;

// ============================================================================
// Texture/Sampler Heaps (same as vertex PBR shader)
// ============================================================================
[[vk::binding(0, 0)]]
SamplerState samplerHeap[];

[[vk::binding(0, 1)]]
Texture2D textureHeap[];

// ============================================================================
// Pixel Shader Data (reuse from PBR shader)
// ============================================================================
struct MaterialData
{
    float4 baseColor;
    float metallic;
    float roughness;
    float ao;
    float _padding0;
    float4 emissive;
};

struct Light
{
    float posX, posY, posZ;
    float range;
    float dirX, dirY, dirZ;
    float spotAngle;
    float colorR, colorG, colorB;
    float intensity;
    uint type;
    uint castShadows;
    float2 _padding;
};

float3 GetLightPosition(Light light) { return float3(light.posX, light.posY, light.posZ); }
float3 GetLightDirection(Light light) { return float3(light.dirX, light.dirY, light.dirZ); }
float3 GetLightColor(Light light) { return float3(light.colorR, light.colorG, light.colorB); }

struct SceneLighting
{
    float camPosX, camPosY, camPosZ;
    uint lightCount;
    float ambientR, ambientG, ambientB;
    float ambientIntensity;
    Light lights[4];
};

float3 GetCameraPosition(SceneLighting scene) { return float3(scene.camPosX, scene.camPosY, scene.camPosZ); }
float3 GetAmbientColor(SceneLighting scene) { return float3(scene.ambientR, scene.ambientG, scene.ambientB); }

struct PixelShaderData
{
    MaterialData* material;
    SceneLighting* scene;
    uint baseColorTexture;
    uint normalMapTexture;
    uint metallicRoughnessTexture;
    uint aoTexture;
    uint emissiveTexture;
    uint samplerIndex;
    uint2 _padding;
};

// ============================================================================
// Mesh Shader Output
// ============================================================================
struct MeshOutput
{
    float4 clipPosition : SV_Position;
    float3 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float3 worldTangent : WORLD_TANGENT;
    float3 worldBitangent : WORLD_BITANGENT;
    float2 texCoord : TEXCOORD0;
};

// ============================================================================
// Mesh Shader
// ============================================================================

// Mesh shader outputs: up to MAX_VERTICES_PER_MESHLET vertices and MAX_PRIMITIVES_PER_MESHLET triangles
// Each thread processes vertices and primitives cooperatively

[outputtopology("triangle")]
[numthreads(MESH_WORKGROUP_SIZE, 1, 1)]
[shader("mesh")]
void meshMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out vertices MeshOutput verts[MAX_VERTICES_PER_MESHLET],
    out indices uint3 tris[MAX_PRIMITIVES_PER_MESHLET])
{
    // Load mesh data from root constants
    MeshShaderData meshData = *root.meshData;
    
    // Get meshlet for this workgroup
    uint meshletIndex = gid;
    if (meshletIndex >= meshData.meshletCount)
    {
        // No meshlet for this workgroup, output nothing
        SetMeshOutputCounts(0, 0);
        return;
    }
    
    Meshlet meshlet = meshData.meshlets[meshletIndex];
    MeshInstanceData instance = *meshData.instance;
    
    // Set output counts (cast uint8_t to uint for API)
    uint vertexCount = uint(meshlet.vertexCount);
    uint triangleCount = uint(meshlet.triangleCount);
    SetMeshOutputCounts(vertexCount, triangleCount);
    
    // Process vertices - each thread handles multiple vertices if needed
    for (uint vi = gtid; vi < vertexCount; vi += MESH_WORKGROUP_SIZE)
    {
        // Get actual vertex index from meshlet's vertex list
        uint vertexIndex = meshData.meshletVertices[meshlet.vertexOffset + vi];
        
        // Load vertex data
        MeshVertex vertex = LoadMeshVertex(meshData.vertices, vertexIndex);
        
        // Transform to clip space
        float4 worldPos = mul(instance.model, float4(vertex.position, 1.0));
        verts[vi].clipPosition = mul(instance.mvp, float4(vertex.position, 1.0));
        verts[vi].worldPosition = worldPos.xyz;
        
        // Transform normal
        float3 worldNormal = mul((float3x3)instance.normalMatrix, vertex.normal);
        verts[vi].worldNormal = normalize(worldNormal);
        
        // Transform tangent
        float3 worldTangent = mul((float3x3)instance.model, vertex.tangent.xyz);
        verts[vi].worldTangent = normalize(worldTangent);
        
        // Compute bitangent
        verts[vi].worldBitangent = cross(verts[vi].worldNormal, verts[vi].worldTangent) * vertex.tangent.w;
        
        // Pass through texture coordinates
        verts[vi].texCoord = vertex.texCoord;
    }
    
    // Synchronize all threads before processing triangles
    // Ensures all vertices are written before any thread writes triangles
    GroupMemoryBarrierWithGroupSync();
    
    // Process triangles - each thread handles multiple triangles if needed
    for (uint ti = gtid; ti < uint(meshlet.triangleCount); ti += MESH_WORKGROUP_SIZE)
    {
        // Primitive indices are stored as packed uint8 bytes (3 bytes per triangle)
        // triangleOffset is already the byte offset into the buffer
        // We read via UnpackTriangle which handles the byte extraction
        uint3 triIndices = UnpackTriangle(meshData.meshletTriangles, meshlet.triangleOffset, ti);
        tris[ti] = triIndices;
    }
}

// ============================================================================
// PBR Helper Functions (same as vertex shader version)
// ============================================================================

static const float PI = 3.14159265359;
static const float EPSILON = 0.0001;
static const uint TEXTURE_INDEX_NONE = 0xFFFFFFFF;

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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / max(denom, EPSILON);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float CalculateAttenuation(float distance, float range)
{
    float att = saturate(1.0 - (distance / range));
    return att * att;
}

// ============================================================================
// Fragment Shader (same as vertex PBR version)
// ============================================================================

[shader("fragment")]
float4 fragmentMain(MeshOutput input) : SV_Target
{
    PixelShaderData psData = *root.pixelData;
    MaterialData material = *psData.material;
    SceneLighting scene = *psData.scene;
    
    float3 albedo = material.baseColor.rgb;
    float metallic = material.metallic;
    float roughness = max(material.roughness, 0.04);
    float ao = material.ao;
    
    uint baseColorTex = psData.baseColorTexture;
    uint normalTex = psData.normalMapTexture;
    uint mrTex = psData.metallicRoughnessTexture;
    uint aoTex = psData.aoTexture;
    uint emissiveTex = psData.emissiveTexture;
    uint samplerIdx = psData.samplerIndex;
    
    if (baseColorTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float4 texColor = textureHeap[baseColorTex].Sample(samplerHeap[samplerIdx], input.texCoord);
        albedo *= texColor.rgb;
    }
    
    float3 N = normalize(input.worldNormal);
    if (normalTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float3 tangentNormal = textureHeap[normalTex].Sample(samplerHeap[samplerIdx], input.texCoord).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        
        float3 T = normalize(input.worldTangent);
        float3 B = normalize(input.worldBitangent);
        float3x3 TBN = float3x3(T, B, N);
        N = normalize(mul(tangentNormal, TBN));
    }
    
    if (mrTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        float4 mr = textureHeap[mrTex].Sample(samplerHeap[samplerIdx], input.texCoord);
        roughness *= mr.g;
        metallic *= mr.b;
    }
    
    if (aoTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        ao *= textureHeap[aoTex].Sample(samplerHeap[samplerIdx], input.texCoord).r;
    }
    
    float3 V = normalize(GetCameraPosition(scene) - input.worldPosition);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    float3 Lo = float3(0, 0, 0);
    
    for (uint i = 0; i < min(scene.lightCount, 4); i++) {
        Light light = scene.lights[i];
        
        float3 L;
        float attenuation = 1.0;
        
        if (light.type == 0) {
            L = normalize(-GetLightDirection(light));
        } else {
            float3 lightVec = GetLightPosition(light) - input.worldPosition;
            float distance = length(lightVec);
            L = lightVec / distance;
            attenuation = CalculateAttenuation(distance, light.range);
            
            if (light.type == 2) {
                float theta = dot(L, normalize(-GetLightDirection(light)));
                float epsilon = light.spotAngle * 0.1;
                attenuation *= saturate((theta - cos(light.spotAngle)) / epsilon);
            }
        }
        
        float3 H = normalize(V + L);
        float3 radiance = GetLightColor(light) * light.intensity * attenuation;
        
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
    
    float3 ambient = GetAmbientColor(scene) * scene.ambientIntensity * albedo * ao;
    
    float3 emissive = material.emissive.rgb * material.emissive.a;
    if (emissiveTex != TEXTURE_INDEX_NONE && samplerIdx < 1024) {
        emissive *= textureHeap[emissiveTex].Sample(samplerHeap[samplerIdx], input.texCoord).rgb;
    }
    
    float3 color = ambient + Lo + emissive;
    
    // HDR tonemapping (Reinhard)
    color = color / (color + 1.0);
    
    // Gamma correction
    color = pow(color, 1.0 / 2.2);
    
    return float4(color, material.baseColor.a);
}
