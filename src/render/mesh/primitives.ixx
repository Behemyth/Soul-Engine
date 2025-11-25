export module synodic.soul.render.mesh:primitives;

import std;
import :vertex;
import :mesh;

// Generate a unit cube centered at origin
// 24 vertices (4 per face for correct normals)
// 36 indices (2 triangles per face)
export inline MeshData GenerateCube(float size = 1.0f) {
	MeshData mesh;
	mesh.vertices.reserve(24);
	mesh.indices.reserve(36);

	float h = size * 0.5f;

	// Front face (+Z)
	mesh.vertices.push_back({{-h, -h,  h}, {0, 0, 1}, {0, 0}});  // 0
	mesh.vertices.push_back({{ h, -h,  h}, {0, 0, 1}, {1, 0}});  // 1
	mesh.vertices.push_back({{ h,  h,  h}, {0, 0, 1}, {1, 1}});  // 2
	mesh.vertices.push_back({{-h,  h,  h}, {0, 0, 1}, {0, 1}});  // 3

	// Back face (-Z)
	mesh.vertices.push_back({{ h, -h, -h}, {0, 0, -1}, {0, 0}}); // 4
	mesh.vertices.push_back({{-h, -h, -h}, {0, 0, -1}, {1, 0}}); // 5
	mesh.vertices.push_back({{-h,  h, -h}, {0, 0, -1}, {1, 1}}); // 6
	mesh.vertices.push_back({{ h,  h, -h}, {0, 0, -1}, {0, 1}}); // 7

	// Top face (+Y)
	mesh.vertices.push_back({{-h,  h,  h}, {0, 1, 0}, {0, 0}});  // 8
	mesh.vertices.push_back({{ h,  h,  h}, {0, 1, 0}, {1, 0}});  // 9
	mesh.vertices.push_back({{ h,  h, -h}, {0, 1, 0}, {1, 1}});  // 10
	mesh.vertices.push_back({{-h,  h, -h}, {0, 1, 0}, {0, 1}});  // 11

	// Bottom face (-Y)
	mesh.vertices.push_back({{-h, -h, -h}, {0, -1, 0}, {0, 0}}); // 12
	mesh.vertices.push_back({{ h, -h, -h}, {0, -1, 0}, {1, 0}}); // 13
	mesh.vertices.push_back({{ h, -h,  h}, {0, -1, 0}, {1, 1}}); // 14
	mesh.vertices.push_back({{-h, -h,  h}, {0, -1, 0}, {0, 1}}); // 15

	// Right face (+X)
	mesh.vertices.push_back({{ h, -h,  h}, {1, 0, 0}, {0, 0}});  // 16
	mesh.vertices.push_back({{ h, -h, -h}, {1, 0, 0}, {1, 0}});  // 17
	mesh.vertices.push_back({{ h,  h, -h}, {1, 0, 0}, {1, 1}});  // 18
	mesh.vertices.push_back({{ h,  h,  h}, {1, 0, 0}, {0, 1}});  // 19

	// Left face (-X)
	mesh.vertices.push_back({{-h, -h, -h}, {-1, 0, 0}, {0, 0}}); // 20
	mesh.vertices.push_back({{-h, -h,  h}, {-1, 0, 0}, {1, 0}}); // 21
	mesh.vertices.push_back({{-h,  h,  h}, {-1, 0, 0}, {1, 1}}); // 22
	mesh.vertices.push_back({{-h,  h, -h}, {-1, 0, 0}, {0, 1}}); // 23

	// Indices (clockwise winding for Vulkan front-face)
	auto addFace = [&mesh](Index a, Index b, Index c, Index d) {
		// First triangle: a, c, b (reversed from a, b, c)
		mesh.indices.push_back(a);
		mesh.indices.push_back(c);
		mesh.indices.push_back(b);
		// Second triangle: a, d, c (reversed from a, c, d)
		mesh.indices.push_back(a);
		mesh.indices.push_back(d);
		mesh.indices.push_back(c);
	};

	addFace(0, 1, 2, 3);     // Front
	addFace(4, 5, 6, 7);     // Back
	addFace(8, 9, 10, 11);   // Top
	addFace(12, 13, 14, 15); // Bottom
	addFace(16, 17, 18, 19); // Right
	addFace(20, 21, 22, 23); // Left

	// Compute tangents for each face
	mesh.ComputeTangents();

	return mesh;
}

// Generate a UV sphere
export inline MeshData GenerateSphere(float radius = 0.5f, std::uint32_t segments = 32, std::uint32_t rings = 16) {
	MeshData mesh;

	// Generate vertices
	for (std::uint32_t ring = 0; ring <= rings; ++ring) {
		float phi = static_cast<float>(ring) / static_cast<float>(rings) * 3.14159265f;
		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);

		for (std::uint32_t seg = 0; seg <= segments; ++seg) {
			float theta = static_cast<float>(seg) / static_cast<float>(segments) * 2.0f * 3.14159265f;
			float sinTheta = std::sin(theta);
			float cosTheta = std::cos(theta);

			vec3 normal{sinPhi * cosTheta, cosPhi, sinPhi * sinTheta};
			vec3 position = normal * radius;
			vec2 uv{
				static_cast<float>(seg) / static_cast<float>(segments),
				static_cast<float>(ring) / static_cast<float>(rings)
			};

			mesh.vertices.push_back({position, normal, uv});
		}
	}

	// Generate indices
	for (std::uint32_t ring = 0; ring < rings; ++ring) {
		for (std::uint32_t seg = 0; seg < segments; ++seg) {
			Index curr = ring * (segments + 1) + seg;
			Index next = curr + segments + 1;

			mesh.indices.push_back(curr);
			mesh.indices.push_back(next);
			mesh.indices.push_back(curr + 1);

			mesh.indices.push_back(curr + 1);
			mesh.indices.push_back(next);
			mesh.indices.push_back(next + 1);
		}
	}

	mesh.ComputeTangents();
	return mesh;
}

// Generate a plane (for ground, walls, etc.)
export inline MeshData GeneratePlane(float width = 1.0f, float depth = 1.0f, std::uint32_t subdivisionsX = 1, std::uint32_t subdivisionsZ = 1) {
	MeshData mesh;

	float hw = width * 0.5f;
	float hd = depth * 0.5f;

	for (std::uint32_t z = 0; z <= subdivisionsZ; ++z) {
		for (std::uint32_t x = 0; x <= subdivisionsX; ++x) {
			float u = static_cast<float>(x) / static_cast<float>(subdivisionsX);
			float v = static_cast<float>(z) / static_cast<float>(subdivisionsZ);

			vec3 pos{u * width - hw, 0.0f, v * depth - hd};
			vec3 normal{0.0f, 1.0f, 0.0f};
			vec2 uv{u, v};

			mesh.vertices.push_back({pos, normal, uv});
		}
	}

	for (std::uint32_t z = 0; z < subdivisionsZ; ++z) {
		for (std::uint32_t x = 0; x < subdivisionsX; ++x) {
			Index curr = z * (subdivisionsX + 1) + x;
			Index next = curr + subdivisionsX + 1;

			mesh.indices.push_back(curr);
			mesh.indices.push_back(next);
			mesh.indices.push_back(curr + 1);

			mesh.indices.push_back(curr + 1);
			mesh.indices.push_back(next);
			mesh.indices.push_back(next + 1);
		}
	}

	mesh.ComputeTangents();
	return mesh;
}
