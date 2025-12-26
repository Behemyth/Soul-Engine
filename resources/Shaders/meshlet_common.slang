// Meshlet Common - Shared data structures for mesh shader rendering
// This file defines the GPU data layouts for meshlet-based rendering
// Uses meshoptimizer format for meshlet generation

#ifndef MESHLET_COMMON_SLANG
#define MESHLET_COMMON_SLANG

// ============================================================================
// Meshlet Data Structures (matches meshoptimizer output)
// ============================================================================

// Meshlet descriptor - describes a single meshlet's vertex/primitive data
// Matches meshopt_Meshlet structure (16 bytes)
struct Meshlet
{
    uint vertexOffset;      // Offset into meshlet vertex indices buffer
    uint triangleOffset;    // Offset into meshlet triangle indices buffer
    uint vertexCount;       // Number of vertices in this meshlet (max 64)
    uint triangleCount;     // Number of triangles in this meshlet (max 124)
};

// Meshlet bounds for culling (matches meshopt_Bounds - 40 bytes)
struct MeshletBounds
{
    float3 center;          // Bounding sphere center
    float  radius;          // Bounding sphere radius
    float3 coneApex;        // Cone apex for backface culling
    float  _padding0;
    float3 coneAxis;        // Cone axis (normalized)
    float  coneCutoff;      // cos(angle) for cone culling
};

// ============================================================================
// Mesh Shader Constants
// ============================================================================

// Maximum vertices/primitives per meshlet (VK_EXT_mesh_shader spec)
// Using conservative values that work across all vendors
static const uint MAX_VERTICES_PER_MESHLET = 64;
static const uint MAX_PRIMITIVES_PER_MESHLET = 124;

// Workgroup size for mesh shaders (one thread per primitive or vertex)
static const uint MESH_WORKGROUP_SIZE = 128;

// Task shader workgroup size (one thread per meshlet batch)
static const uint TASK_WORKGROUP_SIZE = 32;

// ============================================================================
// Mesh Shader Data Blocks (using Slang pointer syntax)
// ============================================================================

// Per-draw instance data for mesh shaders
struct MeshInstanceData
{
    float4x4 mvp;           // Model-View-Projection
    float4x4 model;         // Model matrix for world-space calculations
    float4x4 normalMatrix;  // Inverse transpose of model for correct normal transform
};

// Complete mesh data passed via GPU pointer
// Note: Uses raw float components to avoid vec3 alignment issues with PhysicalStorageBuffer
struct MeshVertexRaw
{
    float posX, posY, posZ;       // 12 bytes (offset 0)
    float normX, normY, normZ;    // 12 bytes (offset 12)
    float tanX, tanY, tanZ, tanW; // 16 bytes (offset 24)
    float2 texCoord;              // 8 bytes (offset 40)
};  // 48 bytes total

// Mesh shader vertex data - passed via root constants
struct MeshShaderData
{
    MeshVertexRaw* vertices;       // GPU pointer to vertex buffer
    uint* meshletVertices;         // GPU pointer to meshlet vertex indices (into vertices)
    uint* meshletTriangles;        // GPU pointer to packed triangle indices (3 bytes per triangle)
    Meshlet* meshlets;             // GPU pointer to meshlet descriptors
    MeshletBounds* meshletBounds;  // GPU pointer to meshlet culling bounds (optional)
    MeshInstanceData* instance;    // GPU pointer to transform data
    uint meshletCount;             // Total number of meshlets
    uint _padding;
};

// Task shader payload - data passed from task to mesh shader
// Contains visible meshlet indices after culling
struct TaskPayload
{
    uint meshletIndices[TASK_WORKGROUP_SIZE];  // Indices of visible meshlets
    uint meshletCount;                          // Number of visible meshlets
};

// ============================================================================
// Helper Functions
// ============================================================================

// Load a vertex from raw buffer format
struct MeshVertex
{
    float3 position;
    float3 normal;
    float4 tangent;
    float2 texCoord;
};

MeshVertex LoadMeshVertex(MeshVertexRaw* buffer, uint index)
{
    MeshVertexRaw raw = buffer[index];
    MeshVertex v;
    v.position = float3(raw.posX, raw.posY, raw.posZ);
    v.normal = float3(raw.normX, raw.normY, raw.normZ);
    v.tangent = float4(raw.tanX, raw.tanY, raw.tanZ, raw.tanW);
    v.texCoord = raw.texCoord;
    return v;
}

// Unpack triangle indices from meshoptimizer's packed format
// Each triangle is stored as 3 consecutive bytes in the buffer
uint3 UnpackTriangle(uint* triangleBuffer, uint triangleOffset, uint triangleIndex)
{
    // meshoptimizer packs triangles as 3 bytes each
    // We read 32-bit words and extract bytes
    uint byteOffset = triangleOffset + triangleIndex * 3;
    uint wordOffset = byteOffset / 4;
    uint byteInWord = byteOffset % 4;
    
    uint word0 = triangleBuffer[wordOffset];
    uint word1 = triangleBuffer[wordOffset + 1];  // May need next word
    
    uint3 indices;
    // Extract 3 consecutive bytes
    uint combined = (word0 >> (byteInWord * 8)) | (word1 << ((4 - byteInWord) * 8));
    indices.x = combined & 0xFF;
    indices.y = (combined >> 8) & 0xFF;
    indices.z = (combined >> 16) & 0xFF;
    
    return indices;
}

// Alternative: Read triangle indices assuming 4-byte aligned storage
// (Simpler but uses more memory - 4 bytes per index)
uint3 ReadTriangleIndices(uint* buffer, uint offset, uint triangleIndex)
{
    uint base = offset + triangleIndex * 3;
    return uint3(buffer[base], buffer[base + 1], buffer[base + 2]);
}

// Frustum culling for meshlet bounds
bool FrustumCullMeshlet(MeshletBounds bounds, float4x4 mvp)
{
    // Transform center to clip space
    float4 center = mul(mvp, float4(bounds.center, 1.0));
    
    // Simple sphere vs frustum test (conservative)
    float radius = bounds.radius;
    
    // Check against all 6 frustum planes
    // For perspective projection, planes in clip space are:
    // left:   center.x + center.w >= -radius
    // right:  center.x - center.w <= radius
    // bottom: center.y + center.w >= -radius
    // top:    center.y - center.w <= radius
    // near:   center.z >= -radius
    // far:    center.z - center.w <= radius
    
    if (center.x + center.w < -radius) return true;  // Left
    if (center.x - center.w > radius) return true;   // Right
    if (center.y + center.w < -radius) return true;  // Bottom
    if (center.y - center.w > radius) return true;   // Top
    if (center.z < -radius) return true;             // Near
    if (center.z - center.w > radius) return true;   // Far
    
    return false;  // Visible
}

// Cone culling for backface meshlets
bool ConeCullMeshlet(MeshletBounds bounds, float3 cameraPosition, float4x4 model)
{
    // Transform cone apex to world space
    float3 worldApex = mul(model, float4(bounds.coneApex, 1.0)).xyz;
    float3 worldAxis = normalize(mul((float3x3)model, bounds.coneAxis));
    
    // Direction from camera to cone apex
    float3 viewDir = normalize(worldApex - cameraPosition);
    
    // Cull if view direction is outside the cone
    return dot(viewDir, worldAxis) >= bounds.coneCutoff;
}

#endif // MESHLET_COMMON_SLANG
