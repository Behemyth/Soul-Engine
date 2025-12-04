# Asset Loading

Soul Engine provides modular asset loading through the transput system with pluggable backends.

## Mesh Loading

### GLTF Backend

Load glTF 2.0 meshes (`.gltf` and `.glb` formats) using the GLTF backend:

```cpp
import synodic.soul.transput;

// Load from file
auto result = synodic::soul::gltf::LoadMesh("resources/assets/box/box.glb");
if (result) {
    MeshData& mesh = result->data;
    AABB& bounds = result->bounds;
    // Upload to GPU via MeshUploader...
}
```

### Supported Features

| Feature | Status |
|---------|--------|
| GLB binary format | ✓ |
| GLTF JSON + embedded base64 | ✓ |
| Positions, normals, UVs | ✓ |
| Tangent generation | ✓ |
| 8/16/32-bit indices | ✓ |
| External .bin files | Planned |
| Multi-primitive meshes | Planned |
| Materials/textures | Planned |
| Scene hierarchy | Planned |

### Load Options

```cpp
synodic::soul::mesh::MeshLoadOptions options;
options.computeTangents = true;   // Generate tangents if missing
options.generateNormals = true;   // Generate flat normals if missing
options.flipTexCoordV = false;    // Flip V for top-left origin textures
options.scale = 1.0f;             // Scale factor for positions

auto result = loader.Load(path, options);
```

## Coordinate System

Soul Engine uses a **right-handed coordinate system with +Y up**:

- **+X**: Right
- **+Y**: Up
- **+Z**: Forward (out of screen toward viewer)

This matches glTF 2.0, so no coordinate transformation is applied during import.

## Adding New Backends

Implement the `MeshLoader` interface in `src/transput/resource/mesh/backend/`:

```cpp
class MyFormatLoader : public mesh::MeshLoader {
    bool SupportsExtension(std::string_view ext) const override;
    MeshResult<LoadedMesh> Load(const path& p, const MeshLoadOptions& opts) override;
    MeshResult<LoadedMesh> LoadFromMemory(span<const byte> data, string_view hint, const MeshLoadOptions& opts) override;
};
```

Enable via CMake option (e.g., `BUILD_MY_FORMAT_LOADER`).

