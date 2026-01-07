/**
 * @brief PBR Cube Viewer
 * 
 * This example demonstrates:
 * - GPU pointer-based rendering
 * - Bindless texture/sampler heaps
 * - Dynamic state for depth/stencil/blend
 * - Stage-only barriers
 */

import std;
import synodic.periapsis;
import synodic.soul.core;
import synodic.soul.engine;
import synodic.soul.scheduler;

import synodic.soul.raster.backend.vulkan;
import synodic.soul.window.backend.sdl;
import synodic.soul.input.backend.sdl;
import synodic.soul.raster.backend.mock;
import synodic.soul.gui.backend.standard;
import synodic.soul.gui;
import synodic.soul.render.graph.backend.standard;
import synodic.soul.render.graph;
import synodic.soul.render.mesh;
import synodic.soul.render.material;
import synodic.soul.compute.backend.mock;
import synodic.soul.scheduler.backend.passthrough;
import synodic.soul.backend.sdl;
import synodic.soul.transput; // GLTF mesh loading
import synodic.soul.raster;   // For CommandList, GPUPointer types

// Render mode enum for dropdown demo
enum class RenderMode : std::int32_t {
	PBR = 0,
	Wireframe = 1,
	Normals = 2,
	Depth = 3
};

// ============================================================================
// GPU Data Structures
// ============================================================================

// Instance transform data (matches pbr.slang InstanceData)
struct alignas(16) GPUInstanceData {
	peri::math::mat4 mvp;           // Model-View-Projection
	peri::math::mat4 model;         // Model matrix for world-space calculations  
	peri::math::mat4 normalMatrix;  // Inverse transpose of model
};
static_assert(sizeof(GPUInstanceData) == 192, "GPUInstanceData must be 192 bytes");

// Material data (matches pbr.slang MaterialData)
struct alignas(16) GPUMaterialData {
	peri::math::vec4 baseColor;     // RGB + alpha
	float metallic;
	float roughness;
	float ao;
	float _padding0;
	peri::math::vec4 emissive;      // RGB + intensity
};
static_assert(sizeof(GPUMaterialData) == 48, "GPUMaterialData must be 48 bytes");

// Light (matches pbr.slang Light) - use explicit layout for GPU compatibility
struct GPULight {
	float positionX, positionY, positionZ;
	float range;
	float directionX, directionY, directionZ;
	float spotAngle;
	float colorR, colorG, colorB;
	float intensity;
	std::uint32_t type;             // 0 = directional, 1 = point, 2 = spot
	std::uint32_t castShadows;
	float _padding[2];
};
static_assert(sizeof(GPULight) == 64, "GPULight must be 64 bytes");

// Scene lighting (matches pbr.slang SceneLighting)
struct GPUSceneLighting {
	float cameraPositionX, cameraPositionY, cameraPositionZ;
	std::uint32_t lightCount;
	float ambientColorR, ambientColorG, ambientColorB;
	float ambientIntensity;
	GPULight lights[4];
};
static_assert(sizeof(GPUSceneLighting) == 288, "GPUSceneLighting must be 288 bytes");

// Vertex shader data (matches pbr.slang VertexShaderData)
struct GPUVertexShaderData {
	GPUDeviceAddress vertexBuffer;   // GPU pointer to PBRVertex[]
	GPUDeviceAddress indexBuffer;    // GPU pointer to uint32[] (0 = non-indexed)
	GPUDeviceAddress instanceData;   // GPU pointer to GPUInstanceData
	std::uint32_t vertexCount;
	std::uint32_t indexCount;
	std::uint32_t instanceId;
	std::uint32_t _padding;
};
static_assert(sizeof(GPUVertexShaderData) == 40, "GPUVertexShaderData must be 40 bytes");

// Pixel shader data (matches pbr.slang PixelShaderData)
struct GPUPixelShaderData {
	GPUDeviceAddress material;       // GPU pointer to GPUMaterialData
	GPUDeviceAddress scene;          // GPU pointer to GPUSceneLighting
	std::uint32_t baseColorTexture;  // Texture heap index (0xFFFFFFFF = none)
	std::uint32_t normalMapTexture;
	std::uint32_t metallicRoughnessTexture;
	std::uint32_t aoTexture;
	std::uint32_t emissiveTexture;
	std::uint32_t samplerIndex;
	std::uint32_t _padding[2];
};
static_assert(sizeof(GPUPixelShaderData) == 48, "GPUPixelShaderData must be 48 bytes");

// ============================================================================
// Mesh Shader GPU Data Structures (must match meshlet_common.slang)
// ============================================================================

// Mesh shader data passed via root constants (matches meshlet_common.slang MeshShaderData)
struct GPUMeshShaderData {
	GPUDeviceAddress vertices;        // GPU pointer to MeshVertexRaw[]
	GPUDeviceAddress meshletVertices; // GPU pointer to meshlet vertex indices
	GPUDeviceAddress meshletTriangles;// GPU pointer to packed triangle indices
	GPUDeviceAddress meshlets;        // GPU pointer to Meshlet[] descriptors
	GPUDeviceAddress meshletBounds;   // GPU pointer to MeshletBounds[] (optional)
	GPUDeviceAddress instance;        // GPU pointer to MeshInstanceData
	std::uint32_t meshletCount;       // Total number of meshlets
	std::uint32_t _padding;
};
static_assert(sizeof(GPUMeshShaderData) == 56, "GPUMeshShaderData must be 56 bytes");

// Type alias for the scheduler
using Scheduler = PassthroughSchedulerBackend;

// Type alias for the App with all backends
using ViewerApp = synodic::soul::App<
	Scheduler,
	MockBackend,
	SDLInputBackend,
	VulkanRasterBackend<Scheduler>,
	StandardRenderGraphBackend,
	SDLWindowBackend,
	StandardGUIBackend>;

class Viewer : public ViewerApp
{
public:
	explicit Viewer(
		const synodic::soul::Parameters& params,
		SDLInputBackend inputBackend,
		SDLWindowBackend windowBackend,
		Scheduler& schedulerBackend) :
		ViewerApp(params, std::move(inputBackend), std::move(windowBackend), 
		          VulkanRasterBackend<Scheduler>(schedulerBackend))
	{
	}

protected:
	void OnInit() override
	{
		// Initialize render graph with raster backend
		GetSoul().RenderGraph().Initialize(&GetSoul().Raster());

		if (GetSoul().Window().has_value())
		{
			WindowParameters winParams;
			winParams.title			= "Soul Engine - PBR Cube (Bindless)";
			winParams.pixelSize		= {1920, 1080};
			winParams.pixelPosition = {100, 100};
			winParams.type			= WindowType::WINDOWED;
			winParams.monitor		= 0;

			SDLWindow& mainWindow = GetSoul().Window()->CreateWindow(winParams);

			if (mainWindow.IsValid())
			{
				// Create Vulkan surface from SDL window
				auto vkSurface = mainWindow.CreateVulkanSurface(GetSoul().Raster().InstanceHandle());

				if (vkSurface != 0)
				{
					// Create surface in raster backend
					surfaceEntity_ = GetSoul().Raster().CreateSurface(
						static_cast<NativeSurfaceHandle>(vkSurface),
						{winParams.pixelSize.x, winParams.pixelSize.y});

					surfaceSize_	 = {winParams.pixelSize.x, winParams.pixelSize.y};
					hasValidSurface_ = true;
				}
			}
		}

		// Load mesh from GLTF file, fallback to procedural cube
		const std::filesystem::path meshPath = "resources/assets/box/box.glb";
		auto loadResult = synodic::soul::gltf::LoadMesh(meshPath);
		if (loadResult)
		{
			cubeMeshData_ = std::move(loadResult->data);
		}
		else
		{
			// Fallback to procedural cube if GLTF loading fails
			cubeMeshData_ = GenerateCube(1.0f);
		}

		// Upload cube mesh as meshlets for mesh shader rendering
		meshUploader_.emplace(GetSoul().Raster());
		cubeGPUMeshlet_ = meshUploader_->UploadMeshletMesh(cubeMeshData_);

		// Create PBR material for the cube (bright red for debugging)
		cubeMaterial_			 = PBRMaterialData {};
		cubeMaterial_.baseColorR = 1.0f;
		cubeMaterial_.baseColorG = 0.2f;
		cubeMaterial_.baseColorB = 0.2f;
		cubeMaterial_.baseColorA = 1.0f;
		cubeMaterial_.metallic	 = 0.0f;  // Dielectric for simpler shading
		cubeMaterial_.roughness	 = 0.5f;
		cubeMaterial_.ao		 = 1.0f;

		// Set up camera
		cameraPosition_ = {0.0f, 1.5f, 3.0f};
		cameraTarget_	= {0.0f, 0.0f, 0.0f};

		// Calculate view and projection matrices
		float aspectRatio = static_cast<float>(surfaceSize_.x) / static_cast<float>(surfaceSize_.y);
		viewMatrix_ = peri::math::LookAt(
			cameraPosition_,
			cameraTarget_,
			peri::math::vec3{0.0f, 1.0f, 0.0f});  // Standard Y-up
		projectionMatrix_ = peri::math::Perspective(
			0.785398f,	// 45 degrees FOV
			aspectRatio,
			0.1f,   // Near plane
			100.0f);  // Far plane
		// Note: Y-flip handled by negative viewport height in Vulkan backend

		// Set up lighting
		sceneLighting_.cameraPositionX	= cameraPosition_.x;
		sceneLighting_.cameraPositionY	= cameraPosition_.y;
		sceneLighting_.cameraPositionZ	= cameraPosition_.z;
		sceneLighting_.lightCount		= 1;
		sceneLighting_.ambientColorR	= 0.2f;
		sceneLighting_.ambientColorG	= 0.2f;
		sceneLighting_.ambientColorB	= 0.2f;
		sceneLighting_.ambientIntensity = 1.0f;

		// Main directional light (direction points from light source toward scene)
		sceneLighting_.lights[0].type		= LightType::Directional;
		sceneLighting_.lights[0].directionX = -0.5f;
		sceneLighting_.lights[0].directionY = -1.0f;  // Pointing downward = light from above
		sceneLighting_.lights[0].directionZ = -0.3f;
		sceneLighting_.lights[0].colorR		= 1.0f;
		sceneLighting_.lights[0].colorG		= 0.95f;
		sceneLighting_.lights[0].colorB		= 0.9f;
		sceneLighting_.lights[0].intensity	= 3.0f;

		// Initialize orbit camera
		orbitYaw_	   = 0.0f;
		orbitPitch_	   = 0.3f;
		orbitDistance_ = 4.0f;
		lastMouseX_	   = 0.0;
		lastMouseY_	   = 0.0;
		isOrbiting_	   = false;

		// Initialize GUI
		InitializeGUI();

		GetSoul().Input().AddMousePositionCallback(
			[this](double x, double y)
			{
				// Forward to GUI for hit-testing
				if (GetSoul().GUI().has_value())
				{
					GetSoul().GUI()->OnMouseMove({x, y});
				}

				if (isOrbiting_)
				{
					double deltaX = x - lastMouseX_;
					double deltaY = y - lastMouseY_;

					const float sensitivity = 0.005f;
					// Standard orbit: drag right = rotate view right (yaw decreases)
					// drag up = tilt view up (pitch increases with screen-space up)
					orbitYaw_	-= static_cast<float>(deltaX) * sensitivity;
					orbitPitch_ += static_cast<float>(deltaY) * sensitivity;

					// Clamp pitch to avoid gimbal lock
					const float maxPitch = 1.5f;
					orbitPitch_ = std::clamp(orbitPitch_, -maxPitch, maxPitch);
				}
				lastMouseX_ = x;
				lastMouseY_ = y;
			});

		GetSoul().Input().AddMouseButtonCallback(
			[this](std::uint32_t button, ButtonState state)
			{
				// Forward to GUI first
				if (GetSoul().GUI().has_value())
				{
					MouseButton uiButton = (button == 1) ? MouseButton::Left :
					                        (button == 2) ? MouseButton::Middle :
					                        (button == 3) ? MouseButton::Right : MouseButton::None;
					GetSoul().GUI()->OnMouseButton(uiButton, state == ButtonState::PRESS, {lastMouseX_, lastMouseY_});
					
					// Don't orbit if GUI wants the mouse
					if (GetSoul().GUI()->WantsMouse()) {
						return;
					}
				}

				// Left mouse button for orbiting
				if (button == 1)
				{
					isOrbiting_ = (state == ButtonState::PRESS);
				}
			});
	}

	void OnUpdate(Frame& current, Frame& previous) override
	{
		GetSoul().Input().Poll();

		if (GetSoul().Window().has_value())
		{
			GetSoul().Window()->Update();

			// Exit if all windows are closed
			if (!GetSoul().Window()->Active())
			{
				GetSoul().SetActive(false);
			}
		}

		// Update GUI
		if (GetSoul().GUI().has_value())
		{
			auto deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::milliseconds(16)); // ~60fps estimate
			GetSoul().GUI()->Update(deltaTime);
		}

		UpdateGameLogic(current, previous);
		RenderFrame(current, previous);
	}

	void OnShutdown() override
	{
		// Destroy GPU meshlet mesh before raster backend
		if (meshUploader_.has_value() && cubeGPUMeshlet_.IsValid())
		{
			meshUploader_->DestroyMeshletMesh(cubeGPUMeshlet_);
		}
		meshUploader_.reset();

		if (hasValidSurface_)
		{
			GetSoul().Raster().RemoveSurface(surfaceEntity_);
		}
	}

	bool ShouldContinue() const override
	{
		return true;
	}

private:
	void InitializeGUI()
	{
		if (!GetSoul().GUI().has_value())
		{
			return;
		}

		auto& gui = *GetSoul().GUI();

		// Set viewport size to match window
		gui.SetViewportSize({static_cast<double>(surfaceSize_.x), static_cast<double>(surfaceSize_.y)});

		// Create a panel for controls in the top-left
		LayoutParams panelParams;
		panelParams.direction = LayoutDirection::Vertical;
		panelParams.spacing = 8.0;
		panelParams.padding = {12.0, 12.0};

		auto& controlPanel = gui.Root().AddWidget<Panel>(panelParams, HashWidgetPath("viewer/controls"));
		controlPanel.SetPosition({20.0, 20.0});
		controlPanel.SetSize({200.0, 100.0});

		// Create render mode dropdown
		auto& renderModeDropdown = controlPanel.AddWidget<Dropdown<RenderMode>>(
			HashWidgetPath("viewer/controls/renderMode"));
		
		renderModeDropdown.SetPlaceholder("Render Mode");
		renderModeDropdown.AddOption("PBR Shaded", RenderMode::PBR);
		renderModeDropdown.AddOption("Wireframe", RenderMode::Wireframe);
		renderModeDropdown.AddOption("View Normals", RenderMode::Normals);
		renderModeDropdown.AddOption("Depth Buffer", RenderMode::Depth);

		// Bind dropdown to render mode state
		renderModeDropdown.BindTo(currentRenderMode_);
		renderModeDropdown.SetSelectedIndex(0);  // Default to PBR

		// Add callback for logging/debugging
		renderModeDropdown.OnSelectionChanged([](std::size_t index, const RenderMode& mode) {
			// TODO: Actually switch render passes based on mode
			(void)index;
			(void)mode;
		});
	}

	void UpdateGameLogic(Frame& current, Frame& previous)
	{
		// Update camera position based on orbit parameters
		float camX = orbitDistance_ * std::cos(orbitPitch_) * std::sin(orbitYaw_);
		float camY = orbitDistance_ * std::sin(orbitPitch_);
		float camZ = orbitDistance_ * std::cos(orbitPitch_) * std::cos(orbitYaw_);

		cameraPosition_ = {camX, camY, camZ};
		cameraTarget_	= {0.0f, 0.0f, 0.0f};

		// Update view matrix
		viewMatrix_ = peri::math::LookAt(
			cameraPosition_,
			cameraTarget_,
			peri::math::vec3{0.0f, 1.0f, 0.0f});  // Standard Y-up

		// Update scene lighting camera position for specular
		sceneLighting_.cameraPositionX = cameraPosition_.x;
		sceneLighting_.cameraPositionY = cameraPosition_.y;
		sceneLighting_.cameraPositionZ = cameraPosition_.z;

		// Model matrix - identity (static cube at origin)
		modelMatrix_ = peri::math::mat4{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		// Compute MVP for push constant/vertex data
		peri::math::mat4 vp = projectionMatrix_ * viewMatrix_;
		pushConstants_.mvp	 = vp * modelMatrix_;
		pushConstants_.model = modelMatrix_;
	}

	void RenderFrame(Frame& current, Frame& previous)
	{
		if (!hasValidSurface_)
		{
			return;
		}

		// NOTE: With bindless pattern, material/lighting data is passed via GPU pointers
		// in the draw command, not via UpdateMaterial/UpdateSceneLighting.
		// For now, these are stub functions - full implementation would use GPUAllocator.
		auto& raster = GetSoul().Raster();
		raster.UpdateMaterial(&cubeMaterial_, sizeof(cubeMaterial_));
		raster.UpdateSceneLighting(&sceneLighting_, sizeof(sceneLighting_));

		auto& renderGraph = GetSoul().RenderGraph();

		// Begin a new frame - clears the previous graph
		renderGraph.BeginFrame();

		// Import the surface as an external resource
		ResourceHandle surfaceResource = renderGraph.ImportSurface(surfaceEntity_);

		// Lambda to record cube draw commands using mesh shaders with meshlets
		auto recordCubeDraw = [this](PassExecutionContext& ctx)
		{
			if (!ctx.allocator) {
				// Fallback if no allocator available
				return;
			}

			// === Mesh Shader Drawing Pattern with Meshlets ===
			
			// 1. Allocate per-draw GPU data for mesh shader
			auto instanceRaw = ctx.allocator->AllocateTyped<GPUInstanceData>();
			auto materialRaw = ctx.allocator->AllocateTyped<GPUMaterialData>();
			auto sceneRaw = ctx.allocator->AllocateTyped<GPUSceneLighting>();
			auto meshDataRaw = ctx.allocator->AllocateTyped<GPUMeshShaderData>();
			auto psDataRaw = ctx.allocator->AllocateTyped<GPUPixelShaderData>();
			
			if (!instanceRaw.IsValid() || !materialRaw.IsValid() || !sceneRaw.IsValid() ||
			    !meshDataRaw.IsValid() || !psDataRaw.IsValid()) {
				// Out of frame allocator memory
				return;
			}
			
			// 2. Fill instance data (transforms)
			auto* instance = instanceRaw.As<GPUInstanceData>();
			instance->mvp = projectionMatrix_ * viewMatrix_ * modelMatrix_;
			instance->model = modelMatrix_;
			instance->normalMatrix = peri::math::Transpose(peri::math::Inverse(modelMatrix_));
			
			// 3. Fill material data
			auto* material = materialRaw.As<GPUMaterialData>();
			material->baseColor = {cubeMaterial_.baseColorR, cubeMaterial_.baseColorG,
			                       cubeMaterial_.baseColorB, cubeMaterial_.baseColorA};
			material->metallic = cubeMaterial_.metallic;
			material->roughness = cubeMaterial_.roughness;
			material->ao = cubeMaterial_.ao;
			material->emissive = {0.0f, 0.0f, 0.0f, 0.0f};
			
			// 4. Fill scene lighting
			auto* scene = sceneRaw.As<GPUSceneLighting>();
			scene->cameraPositionX = cameraPosition_.x;
			scene->cameraPositionY = cameraPosition_.y;
			scene->cameraPositionZ = cameraPosition_.z;
			scene->lightCount = sceneLighting_.lightCount;
			scene->ambientColorR = sceneLighting_.ambientColorR;
			scene->ambientColorG = sceneLighting_.ambientColorG;
			scene->ambientColorB = sceneLighting_.ambientColorB;
			scene->ambientIntensity = sceneLighting_.ambientIntensity;
			
			// Copy first light
			if (sceneLighting_.lightCount > 0) {
				scene->lights[0].positionX = sceneLighting_.lights[0].positionX;
				scene->lights[0].positionY = sceneLighting_.lights[0].positionY;
				scene->lights[0].positionZ = sceneLighting_.lights[0].positionZ;
				scene->lights[0].directionX = sceneLighting_.lights[0].directionX;
				scene->lights[0].directionY = sceneLighting_.lights[0].directionY;
				scene->lights[0].directionZ = sceneLighting_.lights[0].directionZ;
				scene->lights[0].colorR = sceneLighting_.lights[0].colorR;
				scene->lights[0].colorG = sceneLighting_.lights[0].colorG;
				scene->lights[0].colorB = sceneLighting_.lights[0].colorB;
				scene->lights[0].intensity = sceneLighting_.lights[0].intensity;
				scene->lights[0].type = static_cast<std::uint32_t>(sceneLighting_.lights[0].type);
				scene->lights[0].range = sceneLighting_.lights[0].range;
			}
			
			// 5. Fill mesh shader data struct (meshlet-based)
			auto* meshData = meshDataRaw.As<GPUMeshShaderData>();
			meshData->vertices = cubeGPUMeshlet_.vertexBufferGPU;
			meshData->meshletVertices = cubeGPUMeshlet_.vertexIndexBufferGPU;
			meshData->meshletTriangles = cubeGPUMeshlet_.primitiveIndexBufferGPU;
			meshData->meshlets = cubeGPUMeshlet_.meshletBufferGPU;
			meshData->meshletBounds = cubeGPUMeshlet_.boundsBufferGPU;
			meshData->instance = instanceRaw.gpu;
			meshData->meshletCount = cubeGPUMeshlet_.meshletCount;
			meshData->_padding = 0;
			
			// 6. Fill pixel shader data struct
			auto* psData = psDataRaw.As<GPUPixelShaderData>();
			psData->material = materialRaw.gpu;
			psData->scene = sceneRaw.gpu;
			psData->baseColorTexture = 0xFFFFFFFF;  // No texture
			psData->normalMapTexture = 0xFFFFFFFF;
			psData->metallicRoughnessTexture = 0xFFFFFFFF;
			psData->aoTexture = 0xFFFFFFFF;
			psData->emissiveTexture = 0xFFFFFFFF;
			psData->samplerIndex = 0;
			
			// Set depth-stencil state for opaque geometry
			ctx.commands.SetDepthStencilState(DepthStencilState::DepthReadWrite());
			
			// Set blend state for opaque rendering
			ctx.commands.SetBlendState(BlendState::Opaque());
			
			// 7. Draw with mesh shaders!
			// Each workgroup processes one meshlet, so dispatch meshletCount workgroups
			DrawMeshTasksCommand drawCmd;
			drawCmd.meshData = meshDataRaw.gpu;
			drawCmd.pixelData = psDataRaw.gpu;
			drawCmd.groupCountX = cubeGPUMeshlet_.meshletCount;  // One workgroup per meshlet
			drawCmd.groupCountY = 1;
			drawCmd.groupCountZ = 1;
			ctx.commands.DrawMeshTasks(drawCmd);
		};

		// === Depth Pre-Pass ===
		auto depthPrePass = RenderPassBuilder("DepthPrePass")
								.DepthOutput(surfaceResource, ResourceUsage::DepthStencilWrite)
								.ClearDepth(1.0f)
								.Build();

		renderGraph.AddPassWithCallback(std::move(depthPrePass), recordCubeDraw);

		// === Main PBR Pass ===
		auto mainPass = RenderPassBuilder("MainPBRPass")
							.ColorOutput(surfaceResource, ResourceUsage::Present)
							.DepthInput(surfaceResource, ResourceUsage::DepthStencilRead)
							.ClearColor(0.1f, 0.1f, 0.15f, 1.0f)
							.Build();

		renderGraph.AddPassWithCallback(std::move(mainPass), recordCubeDraw);

		// Execute the render graph
		renderGraph.Execute(surfaceEntity_, surfaceSize_);

		// Present the frame
		renderGraph.Present();
	}

	// Window/surface state
	Entity surfaceEntity_;
	peri::math::uvec2 surfaceSize_ = {0, 0};
	bool hasValidSurface_ = false;

	// Mesh data (CPU-side, ready for GPU upload)
	MeshData cubeMeshData_;
	synodic::soul::mesh::GPUMeshletMesh cubeGPUMeshlet_;  // Meshlet-based for mesh shader rendering

	// Mesh uploader (deferred construction after Raster is ready)
	std::optional<MeshUploader> meshUploader_;

	// Material
	PBRMaterialData cubeMaterial_;

	// Scene lighting
	SceneLightingData sceneLighting_;

	// Camera
	peri::math::vec3 cameraPosition_;
	peri::math::vec3 cameraTarget_;

	// Orbit camera controls
	float orbitYaw_		 = 0.0f;
	float orbitPitch_	 = 0.3f;
	float orbitDistance_ = 4.0f;
	double lastMouseX_	 = 0.0;
	double lastMouseY_	 = 0.0;
	bool isOrbiting_	 = false;

	// Animation
	float rotationAngle_ = 0.0f;

	// Transforms
	peri::math::mat4 modelMatrix_;
	peri::math::mat4 viewMatrix_;
	peri::math::mat4 projectionMatrix_;
	PushConstantData pushConstants_;

	// UI state
	RenderMode currentRenderMode_ = RenderMode::PBR;
};

std::int32_t main(std::int32_t, char*[])
{
	const synodic::soul::Parameters appParams;
	SDLBackend backend;
	SDLWindowBackend windowBackend(backend);
	SDLInputBackend inputBackend(backend, windowBackend);

	Property<std::uint32_t> threadCount(1);
	Scheduler schedulerBackend(threadCount);
	Viewer app(appParams, std::move(inputBackend), std::move(windowBackend), schedulerBackend);

	app.Run();

	return 0;
}
