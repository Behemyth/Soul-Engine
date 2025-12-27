/**
 * @file test_meshlet.cpp
 * @brief Unit tests for meshlet generation and data structures
 * 
 * Tests the CPU-side meshlet pipeline including:
 * - Data structure sizes and alignment (GPU compatibility)
 * - Triangle index packing/unpacking logic
 * - Meshlet generation from mesh data
 * - Edge cases and boundary conditions
 */

import std;
import synodic.periapsis;
import synodic.soul.render.mesh;
import synodic.honesty.test;

using namespace honesty::test;
using namespace honesty::test::literals;
using namespace synodic::soul::mesh;

namespace
{
	// ========================================================================
	// CPU-side triangle unpacking (mirrors shader logic for testing)
	// ========================================================================
	
	/**
	 * @brief CPU implementation of triangle unpacking
	 * 
	 * This must match the GPU shader's UnpackTriangle function exactly.
	 * Used to verify triangle data is correctly encoded.
	 */
	struct uint3 {
		std::uint32_t x, y, z;
		
		bool operator==(const uint3& other) const {
			return x == other.x && y == other.y && z == other.z;
		}
	};
	
	uint3 UnpackTriangle(const std::uint8_t* triangleBuffer, std::uint32_t triangleOffset, std::uint32_t triangleIndex)
	{
		// Match shader logic exactly
		std::uint32_t byteOffset = triangleOffset + triangleIndex * 3;
		std::uint32_t wordOffset = byteOffset / 4;
		std::uint32_t byteInWord = byteOffset % 4;
		
		// Read as 32-bit words (as GPU would)
		const std::uint32_t* wordBuffer = reinterpret_cast<const std::uint32_t*>(triangleBuffer);
		std::uint32_t word0 = wordBuffer[wordOffset];
		
		uint3 indices;
		
		if (byteInWord == 0)
		{
			indices.x = word0 & 0xFF;
			indices.y = (word0 >> 8) & 0xFF;
			indices.z = (word0 >> 16) & 0xFF;
		}
		else if (byteInWord == 1)
		{
			indices.x = (word0 >> 8) & 0xFF;
			indices.y = (word0 >> 16) & 0xFF;
			indices.z = (word0 >> 24) & 0xFF;
		}
		else if (byteInWord == 2)
		{
			std::uint32_t word1 = wordBuffer[wordOffset + 1];
			indices.x = (word0 >> 16) & 0xFF;
			indices.y = (word0 >> 24) & 0xFF;
			indices.z = word1 & 0xFF;
		}
		else // byteInWord == 3
		{
			std::uint32_t word1 = wordBuffer[wordOffset + 1];
			indices.x = (word0 >> 24) & 0xFF;
			indices.y = word1 & 0xFF;
			indices.z = (word1 >> 8) & 0xFF;
		}
		
		return indices;
	}

	// ========================================================================
	// Helper to create simple mesh data for testing
	// ========================================================================
	
	/**
	 * @brief Creates a simple quad mesh (2 triangles)
	 */
	auto CreateQuadMesh() -> MeshData
	{
		MeshData mesh;
		
		// PBR vertex layout: position (3) + normal (3) + tangent (4) + texcoord (2) = 12 floats
		mesh.layout = PBRVertexLayout();  // Use the standard PBR layout
		
		// 4 vertices for a quad
		mesh.vertexCount = 4;
		mesh.vertexData.resize(4 * 12);  // 4 vertices * 12 floats
		
		// Vertex 0: (-1, -1, 0)
		mesh.vertexData[0] = -1.0f; mesh.vertexData[1] = -1.0f; mesh.vertexData[2] = 0.0f;
		mesh.vertexData[3] = 0.0f; mesh.vertexData[4] = 0.0f; mesh.vertexData[5] = 1.0f;  // normal
		mesh.vertexData[6] = 1.0f; mesh.vertexData[7] = 0.0f; mesh.vertexData[8] = 0.0f; mesh.vertexData[9] = 1.0f;  // tangent
		mesh.vertexData[10] = 0.0f; mesh.vertexData[11] = 0.0f;  // texcoord
		
		// Vertex 1: (1, -1, 0)
		mesh.vertexData[12] = 1.0f; mesh.vertexData[13] = -1.0f; mesh.vertexData[14] = 0.0f;
		mesh.vertexData[15] = 0.0f; mesh.vertexData[16] = 0.0f; mesh.vertexData[17] = 1.0f;
		mesh.vertexData[18] = 1.0f; mesh.vertexData[19] = 0.0f; mesh.vertexData[20] = 0.0f; mesh.vertexData[21] = 1.0f;
		mesh.vertexData[22] = 1.0f; mesh.vertexData[23] = 0.0f;
		
		// Vertex 2: (1, 1, 0)
		mesh.vertexData[24] = 1.0f; mesh.vertexData[25] = 1.0f; mesh.vertexData[26] = 0.0f;
		mesh.vertexData[27] = 0.0f; mesh.vertexData[28] = 0.0f; mesh.vertexData[29] = 1.0f;
		mesh.vertexData[30] = 1.0f; mesh.vertexData[31] = 0.0f; mesh.vertexData[32] = 0.0f; mesh.vertexData[33] = 1.0f;
		mesh.vertexData[34] = 1.0f; mesh.vertexData[35] = 1.0f;
		
		// Vertex 3: (-1, 1, 0)
		mesh.vertexData[36] = -1.0f; mesh.vertexData[37] = 1.0f; mesh.vertexData[38] = 0.0f;
		mesh.vertexData[39] = 0.0f; mesh.vertexData[40] = 0.0f; mesh.vertexData[41] = 1.0f;
		mesh.vertexData[42] = 1.0f; mesh.vertexData[43] = 0.0f; mesh.vertexData[44] = 0.0f; mesh.vertexData[45] = 1.0f;
		mesh.vertexData[46] = 0.0f; mesh.vertexData[47] = 1.0f;
		
		// 2 triangles: (0,1,2) and (0,2,3)
		mesh.indices = {0, 1, 2, 0, 2, 3};
		
		return mesh;
	}
	
	/**
	 * @brief Creates a larger mesh for testing multi-meshlet scenarios
	 * 
	 * Creates a grid of triangles that will span multiple meshlets
	 */
	auto CreateGridMesh(std::uint32_t gridSize) -> MeshData
	{
		MeshData mesh;
		
		mesh.layout = PBRVertexLayout();  // Use the standard PBR layout
		
		// (gridSize+1)^2 vertices
		std::uint32_t verticesPerRow = gridSize + 1;
		mesh.vertexCount = verticesPerRow * verticesPerRow;
		mesh.vertexData.resize(mesh.vertexCount * 12);
		
		for (std::uint32_t y = 0; y <= gridSize; ++y)
		{
			for (std::uint32_t x = 0; x <= gridSize; ++x)
			{
				std::uint32_t vi = (y * verticesPerRow + x) * 12;
				float fx = static_cast<float>(x) / gridSize;
				float fy = static_cast<float>(y) / gridSize;
				
				// Position
				mesh.vertexData[vi + 0] = fx * 2.0f - 1.0f;
				mesh.vertexData[vi + 1] = fy * 2.0f - 1.0f;
				mesh.vertexData[vi + 2] = 0.0f;
				
				// Normal (up)
				mesh.vertexData[vi + 3] = 0.0f;
				mesh.vertexData[vi + 4] = 0.0f;
				mesh.vertexData[vi + 5] = 1.0f;
				
				// Tangent
				mesh.vertexData[vi + 6] = 1.0f;
				mesh.vertexData[vi + 7] = 0.0f;
				mesh.vertexData[vi + 8] = 0.0f;
				mesh.vertexData[vi + 9] = 1.0f;
				
				// TexCoord
				mesh.vertexData[vi + 10] = fx;
				mesh.vertexData[vi + 11] = fy;
			}
		}
		
		// 2 triangles per grid cell
		mesh.indices.reserve(gridSize * gridSize * 6);
		for (std::uint32_t y = 0; y < gridSize; ++y)
		{
			for (std::uint32_t x = 0; x < gridSize; ++x)
			{
				std::uint32_t tl = y * verticesPerRow + x;
				std::uint32_t tr = tl + 1;
				std::uint32_t bl = tl + verticesPerRow;
				std::uint32_t br = bl + 1;
				
				// Triangle 1: tl, bl, tr
				mesh.indices.push_back(tl);
				mesh.indices.push_back(bl);
				mesh.indices.push_back(tr);
				
				// Triangle 2: tr, bl, br
				mesh.indices.push_back(tr);
				mesh.indices.push_back(bl);
				mesh.indices.push_back(br);
			}
		}
		
		return mesh;
	}

	// ========================================================================
	// Test Suite
	// ========================================================================
	
	Suite SUITE(
		"meshlet",
		[](const Fixture& fixture) -> Generator
		{
			// ----------------------------------------------------------------
			// Data Structure Tests
			// ----------------------------------------------------------------
			
			co_yield "meshlet_struct_size"_test = [&](const Requirements& requirements)
			{
				// Meshlet must be exactly 12 bytes for GPU compatibility
				requirements.Expect(sizeof(Meshlet) == 12);
				requirements.Expect(alignof(Meshlet) == 4);
			};
			
			co_yield "meshlet_bounds_struct_size"_test = [&](const Requirements& requirements)
			{
				// MeshletBounds must be exactly 32 bytes
				requirements.Expect(sizeof(MeshletBounds) == 32);
			};
			
			co_yield "meshlet_constants"_test = [&](const Requirements& requirements)
			{
				// Verify shader-compatible constants
				requirements.Expect(MaxVerticesPerMeshlet == 64);
				requirements.Expect(MaxPrimitivesPerMeshlet == 124);
			};
			
			// ----------------------------------------------------------------
			// Triangle Unpacking Tests
			// ----------------------------------------------------------------
			
			co_yield "unpack_triangle_byte_aligned"_test = [&](const Requirements& requirements)
			{
				// Test unpacking when triangle starts at byte 0 (word-aligned)
				// Triangle 0 at offset 0: bytes 0,1,2
				std::vector<std::uint8_t> buffer = {
					10, 20, 30,  // Triangle 0: vertices 10, 20, 30
					40, 50, 60,  // Triangle 1
					0, 0         // Padding to 4-byte alignment
				};
				
				auto tri = UnpackTriangle(buffer.data(), 0, 0);
				requirements.Expect(tri.x == 10);
				requirements.Expect(tri.y == 20);
				requirements.Expect(tri.z == 30);
			};
			
			co_yield "unpack_triangle_offset_1"_test = [&](const Requirements& requirements)
			{
				// Test unpacking when triangle starts at byte 1
				std::vector<std::uint8_t> buffer = {
					0,           // Padding
					10, 20, 30,  // Triangle 0 at byte offset 1
					0, 0, 0, 0   // Padding
				};
				
				auto tri = UnpackTriangle(buffer.data(), 1, 0);
				requirements.Expect(tri.x == 10);
				requirements.Expect(tri.y == 20);
				requirements.Expect(tri.z == 30);
			};
			
			co_yield "unpack_triangle_crosses_word_boundary"_test = [&](const Requirements& requirements)
			{
				// Test when triangle crosses a 32-bit word boundary
				// Triangle at byte 2: spans bytes 2,3 (word 0) and byte 0 (word 1)
				std::vector<std::uint8_t> buffer = {
					0, 0,        // Padding (bytes 0,1)
					10, 20,      // First 2 bytes of triangle (bytes 2,3)
					30, 0, 0, 0  // Third byte of triangle + padding
				};
				
				auto tri = UnpackTriangle(buffer.data(), 2, 0);
				requirements.Expect(tri.x == 10);
				requirements.Expect(tri.y == 20);
				requirements.Expect(tri.z == 30);
			};
			
			co_yield "unpack_triangle_offset_3"_test = [&](const Requirements& requirements)
			{
				// Test when triangle starts at byte 3 (crosses word boundary significantly)
				std::vector<std::uint8_t> buffer = {
					0, 0, 0,     // Padding (bytes 0,1,2)
					10,          // First byte of triangle (byte 3 of word 0)
					20, 30,      // Remaining bytes (word 1)
					0, 0         // Padding
				};
				
				auto tri = UnpackTriangle(buffer.data(), 3, 0);
				requirements.Expect(tri.x == 10);
				requirements.Expect(tri.y == 20);
				requirements.Expect(tri.z == 30);
			};
			
			co_yield "unpack_multiple_triangles"_test = [&](const Requirements& requirements)
			{
				// Test unpacking multiple consecutive triangles
				std::vector<std::uint8_t> buffer = {
					0, 1, 2,     // Triangle 0
					3, 4, 5,     // Triangle 1
					6, 7, 8,     // Triangle 2
					9, 10, 11,   // Triangle 3
					0, 0, 0, 0   // Padding to align
				};
				
				auto tri0 = UnpackTriangle(buffer.data(), 0, 0);
				requirements.Expect(tri0.x == 0 && tri0.y == 1 && tri0.z == 2);
				
				auto tri1 = UnpackTriangle(buffer.data(), 0, 1);
				requirements.Expect(tri1.x == 3 && tri1.y == 4 && tri1.z == 5);
				
				auto tri2 = UnpackTriangle(buffer.data(), 0, 2);
				requirements.Expect(tri2.x == 6 && tri2.y == 7 && tri2.z == 8);
				
				auto tri3 = UnpackTriangle(buffer.data(), 0, 3);
				requirements.Expect(tri3.x == 9 && tri3.y == 10 && tri3.z == 11);
			};
			
			co_yield "unpack_triangle_max_vertex_index"_test = [&](const Requirements& requirements)
			{
				// Test that max vertex index (63) is handled correctly
				std::vector<std::uint8_t> buffer = {
					63, 62, 61,  // Max indices
					0            // Padding
				};
				
				auto tri = UnpackTriangle(buffer.data(), 0, 0);
				requirements.Expect(tri.x == 63);
				requirements.Expect(tri.y == 62);
				requirements.Expect(tri.z == 61);
			};
			
			// ----------------------------------------------------------------
			// Meshlet Generation Tests
			// ----------------------------------------------------------------
			
			co_yield "generate_meshlets_simple_quad"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateQuadMesh();
				requirements.Expect(mesh.IsValid());
				
				auto meshletData = GenerateMeshlets(mesh);
				
				// A simple quad should produce exactly 1 meshlet
				requirements.Expect(meshletData.IsValid());
				requirements.Expect(meshletData.MeshletCount() >= 1);
				
				// Should have valid bounds
				requirements.Expect(meshletData.bounds.size() == meshletData.meshlets.size());
				
				// First meshlet should have reasonable data
				const auto& m = meshletData.meshlets[0];
				requirements.Expect(m.vertexCount > 0);
				requirements.Expect(m.triangleCount > 0);
				requirements.Expect(m.vertexCount <= MaxVerticesPerMeshlet);
				requirements.Expect(m.triangleCount <= MaxPrimitivesPerMeshlet);
			};
			
			co_yield "generate_meshlets_multi_meshlet"_test = [&](const Requirements& requirements)
			{
				// Create a grid large enough to require multiple meshlets
				// 64 vertices per meshlet, so a 20x20 grid (441 vertices, 800 triangles)
				// should produce multiple meshlets
				auto mesh = CreateGridMesh(20);
				requirements.Expect(mesh.IsValid());
				
				auto meshletData = GenerateMeshlets(mesh);
				
				requirements.Expect(meshletData.IsValid());
				// With 800 triangles and max 124 per meshlet, expect at least 7 meshlets
				requirements.Expect(meshletData.MeshletCount() >= 7);
				
				// Verify all meshlets have valid data
				for (const auto& m : meshletData.meshlets)
				{
					requirements.Expect(m.vertexCount > 0);
					requirements.Expect(m.triangleCount > 0);
					requirements.Expect(m.vertexCount <= MaxVerticesPerMeshlet);
					requirements.Expect(m.triangleCount <= MaxPrimitivesPerMeshlet);
				}
			};
			
			co_yield "generate_meshlets_preserves_triangles"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateQuadMesh();
				auto meshletData = GenerateMeshlets(mesh);
				
				// Count total triangles across all meshlets
				std::uint32_t totalTriangles = 0;
				for (const auto& m : meshletData.meshlets)
				{
					totalTriangles += m.triangleCount;
				}
				
				// Should have exactly 2 triangles (the quad)
				requirements.Expect(totalTriangles == 2);
			};
			
			co_yield "generate_meshlets_bounds_valid"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateQuadMesh();
				auto meshletData = GenerateMeshlets(mesh);
				
				requirements.Expect(!meshletData.bounds.empty());
				
				const auto& bounds = meshletData.bounds[0];
				
				// Radius should be positive
				requirements.Expect(bounds.radius > 0.0f);
				
				// Cone cutoff should be in valid range [-1, 1]
				requirements.Expect(bounds.coneCutoff >= -1.0f);
				requirements.Expect(bounds.coneCutoff <= 1.0f);
			};
			
			// ----------------------------------------------------------------
			// Edge Cases
			// ----------------------------------------------------------------
			
			co_yield "generate_meshlets_empty_mesh"_test = [&](const Requirements& requirements)
			{
				MeshData emptyMesh;
				auto meshletData = GenerateMeshlets(emptyMesh);
				
				// Empty mesh should produce invalid/empty meshlet data
				requirements.Expect(!meshletData.IsValid());
			};
			
			co_yield "generate_meshlets_single_triangle"_test = [&](const Requirements& requirements)
			{
				MeshData mesh;
				mesh.layout = PBRVertexLayout();
				
				mesh.vertexCount = 3;
				mesh.vertexData = {
					// Vertex 0: position, normal, tangent, texcoord
					0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
					// Vertex 1
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
					// Vertex 2
					0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f,
				};
				mesh.indices = {0, 1, 2};
				
				auto meshletData = GenerateMeshlets(mesh);
				
				requirements.Expect(meshletData.IsValid());
				requirements.Expect(meshletData.MeshletCount() == 1);
				requirements.Expect(meshletData.meshlets[0].triangleCount == 1);
			};
			
			co_yield "meshlet_data_gpu_sizes"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateGridMesh(10);
				auto meshletData = GenerateMeshlets(mesh);
				
				// Verify GPU size calculations
				requirements.Expect(meshletData.MeshletBufferSize() == meshletData.meshlets.size() * 12);
				requirements.Expect(meshletData.BoundsBufferSize() == meshletData.bounds.size() * 32);
				requirements.Expect(meshletData.VertexIndexBufferSize() == meshletData.vertexIndices.size() * 4);
				
				// Primitive buffer should be 4-byte aligned
				requirements.Expect(meshletData.PrimitiveIndexBufferSize() % 4 == 0);
				
				// Total should be sum of parts
				requirements.Expect(meshletData.TotalGPUSize() == 
					meshletData.MeshletBufferSize() + 
					meshletData.BoundsBufferSize() + 
					meshletData.VertexIndexBufferSize() + 
					meshletData.PrimitiveIndexBufferSize());
			};
			
			// ----------------------------------------------------------------
			// Triangle Index Integrity
			// ----------------------------------------------------------------
			
			co_yield "meshlet_triangle_indices_in_range"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateGridMesh(10);
				auto meshletData = GenerateMeshlets(mesh);
				
				// Verify all triangle indices are within the meshlet's vertex range
				for (const auto& m : meshletData.meshlets)
				{
					for (std::uint32_t ti = 0; ti < m.triangleCount; ++ti)
					{
						auto tri = UnpackTriangle(
							meshletData.primitiveIndices.data(), 
							m.triangleOffset, 
							ti);
						
						// Triangle indices are local to meshlet, must be < vertexCount
						requirements.Expect(tri.x < m.vertexCount);
						requirements.Expect(tri.y < m.vertexCount);
						requirements.Expect(tri.z < m.vertexCount);
					}
				}
			};
			
			co_yield "meshlet_vertex_indices_in_range"_test = [&](const Requirements& requirements)
			{
				auto mesh = CreateGridMesh(10);
				auto meshletData = GenerateMeshlets(mesh);
				
				// Verify all vertex indices point to valid vertices in original mesh
				for (const auto& m : meshletData.meshlets)
				{
					for (std::uint32_t vi = 0; vi < m.vertexCount; ++vi)
					{
						std::uint32_t globalIndex = meshletData.vertexIndices[m.vertexOffset + vi];
						requirements.Expect(globalIndex < mesh.vertexCount);
					}
				}
			};
		}
	);
	
} // namespace
