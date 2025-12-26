// PBR Task Shader for Soul Engine
// Task shader for GPU-driven meshlet culling before mesh shader processing
// Works with pbr_mesh.slang for complete task+mesh+fragment pipeline

#include "meshlet_common.slang"

// ============================================================================
// Root Constants - Same as mesh shader
// ============================================================================
struct PixelShaderData;  // Forward declaration

struct RootConstants
{
    MeshShaderData* meshData;
    PixelShaderData* pixelData;
};

[[vk::push_constant]]
ConstantBuffer<RootConstants> root;

// ============================================================================
// Task Shader Output Payload
// ============================================================================
struct TaskOutput
{
    uint meshletIndices[TASK_WORKGROUP_SIZE];
    uint meshletCount;
};

// ============================================================================
// Task Shader - Per-meshlet culling
// ============================================================================

// Shared memory for compaction (must be at module scope in Slang)
groupshared uint s_meshletIndices[TASK_WORKGROUP_SIZE];
groupshared uint s_meshletCount;

[numthreads(TASK_WORKGROUP_SIZE, 1, 1)]
[shader("amplification")]
void taskMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID)
{
    // Load mesh data
    MeshShaderData meshData = *root.meshData;
    MeshInstanceData instance = *meshData.instance;
    
    // Calculate which meshlet this thread processes
    uint meshletIndex = gid * TASK_WORKGROUP_SIZE + gtid;
    
    // Initialize shared counter
    if (gtid == 0)
    {
        s_meshletCount = 0;
    }
    GroupMemoryBarrierWithGroupSync();
    
    // Check if this meshlet is valid and visible
    bool visible = false;
    if (meshletIndex < meshData.meshletCount)
    {
        visible = true;
        
        // Optional: Frustum and cone culling if bounds are available
        if (meshData.meshletBounds != nullptr)
        {
            MeshletBounds bounds = meshData.meshletBounds[meshletIndex];
            
            // Frustum culling
            if (FrustumCullMeshlet(bounds, instance.mvp))
            {
                visible = false;
            }
            
            // Cone culling for backface meshlets (disabled for now - needs camera position)
            // TODO: Pass camera position in mesh data for cone culling
        }
    }
    
    // Compact visible meshlets using wave intrinsics
    if (visible)
    {
        uint index;
        InterlockedAdd(s_meshletCount, 1, index);
        s_meshletIndices[index] = meshletIndex;
    }
    
    GroupMemoryBarrierWithGroupSync();
    
    // Dispatch mesh shader for visible meshlets
    uint visibleCount = s_meshletCount;
    
    // Output payload with visible meshlet indices
    TaskOutput payload;
    if (gtid < visibleCount)
    {
        payload.meshletIndices[gtid] = s_meshletIndices[gtid];
    }
    payload.meshletCount = visibleCount;
    
    // Dispatch mesh shader workgroups for visible meshlets
    // Each visible meshlet gets one mesh shader workgroup
    DispatchMesh(visibleCount, 1, 1, payload);
}

// ============================================================================
// Mesh Shader (task-enabled variant)
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

[outputtopology("triangle")]
[numthreads(MESH_WORKGROUP_SIZE, 1, 1)]
[shader("mesh")]
void meshMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    in payload TaskOutput taskPayload,
    out vertices MeshOutput verts[MAX_VERTICES_PER_MESHLET],
    out indices uint3 tris[MAX_PRIMITIVES_PER_MESHLET])
{
    // Load mesh data from root constants
    MeshShaderData meshData = *root.meshData;
    
    // Get meshlet index from task shader payload
    if (gid >= taskPayload.meshletCount)
    {
        SetMeshOutputCounts(0, 0);
        return;
    }
    
    uint meshletIndex = taskPayload.meshletIndices[gid];
    Meshlet meshlet = meshData.meshlets[meshletIndex];
    MeshInstanceData instance = *meshData.instance;
    
    // Set output counts
    SetMeshOutputCounts(meshlet.vertexCount, meshlet.triangleCount);
    
    // Process vertices
    for (uint vi = gtid; vi < meshlet.vertexCount; vi += MESH_WORKGROUP_SIZE)
    {
        uint vertexIndex = meshData.meshletVertices[meshlet.vertexOffset + vi];
        MeshVertex vertex = LoadMeshVertex(meshData.vertices, vertexIndex);
        
        float4 worldPos = mul(instance.model, float4(vertex.position, 1.0));
        verts[vi].clipPosition = mul(instance.mvp, float4(vertex.position, 1.0));
        verts[vi].worldPosition = worldPos.xyz;
        
        float3 worldNormal = mul((float3x3)instance.normalMatrix, vertex.normal);
        verts[vi].worldNormal = normalize(worldNormal);
        
        float3 worldTangent = mul((float3x3)instance.model, vertex.tangent.xyz);
        verts[vi].worldTangent = normalize(worldTangent);
        
        verts[vi].worldBitangent = cross(verts[vi].worldNormal, verts[vi].worldTangent) * vertex.tangent.w;
        verts[vi].texCoord = vertex.texCoord;
    }
    
    // Process triangles
    for (uint ti = gtid; ti < meshlet.triangleCount; ti += MESH_WORKGROUP_SIZE)
    {
        uint baseIdx = meshlet.triangleOffset + ti * 3;
        uint i0 = meshData.meshletTriangles[baseIdx + 0] & 0xFF;
        uint i1 = meshData.meshletTriangles[baseIdx + 1] & 0xFF;
        uint i2 = meshData.meshletTriangles[baseIdx + 2] & 0xFF;
        tris[ti] = uint3(i0, i1, i2);
    }
}
