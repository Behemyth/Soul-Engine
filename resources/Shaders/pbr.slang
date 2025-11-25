// PBR Shader for Soul Engine
// Implements Cook-Torrance BRDF with GGX/Schlick approximations

// Vertex input - matches PBRVertex layout (48 bytes)
struct VertexInput
{
    float3 position : POSITION;   // location 0
    float3 normal   : NORMAL;     // location 1
    float4 tangent  : TANGENT;    // location 2 (w = handedness)
    float2 texCoord : TEXCOORD0;  // location 3
};

struct VertexOutput
{
    float4 clipPosition : SV_Position;
    float3 worldPosition : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float3 worldTangent : WORLD_TANGENT;
    float3 worldBitangent : WORLD_BITANGENT;
    float2 texCoord : TEXCOORD0;
};

// Push constants (128 bytes)
struct PushConstants
{
    float4x4 mvp;    // Model-View-Projection
    float4x4 model;  // Model matrix for lighting calculations
};

[[vk::push_constant]]
PushConstants pushConstants;

// Material uniform buffer (binding 0, set 0)
struct MaterialData
{
    float4 baseColor;      // RGB + alpha
    float metallic;
    float roughness;
    float ao;
    float _padding0;
    float4 emissive;       // RGB + intensity
};

[[vk::binding(0, 0)]]
ConstantBuffer<MaterialData> material;

// Scene lighting uniform buffer (binding 1, set 0)
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

struct SceneLighting
{
    float3 cameraPosition;
    uint lightCount;
    float3 ambientColor;
    float ambientIntensity;
    Light lights[4];
};

[[vk::binding(1, 0)]]
ConstantBuffer<SceneLighting> scene;

// Constants
static const float PI = 3.14159265359;
static const float EPSILON = 0.0001;

// ============================================================================
// Vertex Shader
// ============================================================================

[shader("vertex")]
VertexOutput vertexMain(VertexInput input)
{
    VertexOutput output;

    // Transform position
    float4 worldPos = mul(pushConstants.model, float4(input.position, 1.0));
    output.worldPosition = worldPos.xyz;
    output.clipPosition = mul(pushConstants.mvp, float4(input.position, 1.0));

    // Transform normal (using upper-left 3x3 of model matrix)
    // For proper normal transformation, should use inverse transpose
    // but for uniform scale this is sufficient
    float3x3 normalMatrix = (float3x3)pushConstants.model;
    output.worldNormal = normalize(mul(normalMatrix, input.normal));
    output.worldTangent = normalize(mul(normalMatrix, input.tangent.xyz));

    // Compute bitangent with handedness
    output.worldBitangent = cross(output.worldNormal, output.worldTangent) * input.tangent.w;

    output.texCoord = input.texCoord;

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
    // Smooth attenuation that reaches zero at range
    float att = saturate(1.0 - (distance / range));
    return att * att;
}

// ============================================================================
// Fragment Shader
// ============================================================================

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target
{
    // Material properties
    float3 albedo = material.baseColor.rgb;
    float metallic = material.metallic;
    float roughness = max(material.roughness, 0.04); // Prevent divide by zero
    float ao = material.ao;

    // Surface vectors
    float3 N = normalize(input.worldNormal);
    float3 V = normalize(scene.cameraPosition - input.worldPosition);

    // Calculate F0 (surface reflection at zero incidence)
    // Dielectrics use 0.04, metals use albedo
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    // Accumulate lighting
    float3 Lo = float3(0.0, 0.0, 0.0);

    for (uint i = 0; i < min(scene.lightCount, 4u); ++i)
    {
        Light light = scene.lights[i];
        float3 L;
        float attenuation = 1.0;

        if (light.type == 0) // Directional
        {
            L = normalize(-light.direction);
        }
        else if (light.type == 1) // Point
        {
            float3 lightVec = light.position - input.worldPosition;
            float distance = length(lightVec);
            L = lightVec / distance;
            attenuation = CalculateAttenuation(distance, light.range);
        }
        else // Spot
        {
            float3 lightVec = light.position - input.worldPosition;
            float distance = length(lightVec);
            L = lightVec / distance;

            float theta = dot(L, normalize(-light.direction));
            float epsilon = 0.1; // Soft edge
            float spotFactor = saturate((theta - cos(light.spotAngle)) / epsilon);

            attenuation = CalculateAttenuation(distance, light.range) * spotFactor;
        }

        float3 H = normalize(V + L);
        float3 radiance = light.color * light.intensity * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        // Specular
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
        float3 specular = numerator / denominator;

        // Energy conservation
        float3 kS = F;
        float3 kD = (1.0 - kS) * (1.0 - metallic);

        // Lambertian diffuse
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // Ambient lighting (simple approximation)
    float3 ambient = scene.ambientColor * scene.ambientIntensity * albedo * ao;

    // Final color
    float3 color = ambient + Lo;

    // Emissive
    color += material.emissive.rgb * material.emissive.w;

    // HDR tonemapping (Reinhard)
    color = color / (color + 1.0);

    // Gamma correction (linear to sRGB)
    color = pow(color, 1.0 / 2.2);

    return float4(color, material.baseColor.a);
}
