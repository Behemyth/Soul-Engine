import std;
import synodic.library;
import synodic.soul.core;
import synodic.soul.engine;
import synodic.soul.scheduler;

import synodic.soul.raster.backend.vulkan;
import synodic.soul.window.backend.sdl;
import synodic.soul.input.backend.sdl;
import synodic.soul.raster.backend.mock;
import synodic.soul.gui.backend.standard;
import synodic.soul.render.graph.backend.standard;
import synodic.soul.render.graph;
import synodic.soul.render.mesh;
import synodic.soul.render.material;
import synodic.soul.compute.backend.mock;
import synodic.soul.scheduler.backend.passthrough;
import synodic.soul.backend.sdl;
import synodic.soul.transput; // GLTF mesh loading

using SampleApp = synodic::soul::App<
	PassthroughSchedulerBackend,
	MockBackend,
	SDLInputBackend,
	VulkanRasterBackend<PassthroughSchedulerBackend>,
	StandardRenderGraphBackend,
	SDLWindowBackend,
	StandardGUIBackend>;

class Sample : public SampleApp
{
public:
	explicit Sample(
		const synodic::soul::Parameters& params,
		SDLInputBackend inputBackend,
		SDLWindowBackend windowBackend,
		PassthroughSchedulerBackend& schedulerBackend) :
		SampleApp(params, std::move(inputBackend), std::move(windowBackend), VulkanRasterBackend(schedulerBackend))
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
			winParams.title			= "Soul Engine - PBR Cube";
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
		auto loadResult						 = synodic::soul::gltf::LoadMesh(meshPath);
		if (loadResult)
		{
			cubeMeshData_ = std::move(loadResult->data);
		}
		else
		{
			// Fallback to procedural cube if GLTF loading fails
			cubeMeshData_ = GenerateCube(1.0f);
		}

		// Upload cube mesh to GPU
		meshUploader_.emplace(GetSoul().Raster());
		cubeGPUMesh_ = meshUploader_->UploadMesh(cubeMeshData_);

		// Create PBR material for the cube (bright red for debugging)
		cubeMaterial_			 = PBRMaterialData {};	// Start fresh
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
		viewMatrix_		  = synodic::math::LookAt(
			cameraPosition_,
			cameraTarget_,
			synodic::math::vec3{0.0f, 1.0f, 0.0f});
		projectionMatrix_ = synodic::math::Perspective(
			0.785398f,	// 45 degrees FOV
			aspectRatio,
			0.1f,  // Near plane
			100.0f);  // Far plane

		// Set up lighting
		sceneLighting_.cameraPositionX	= cameraPosition_.x;
		sceneLighting_.cameraPositionY	= cameraPosition_.y;
		sceneLighting_.cameraPositionZ	= cameraPosition_.z;
		sceneLighting_.lightCount		= 1;
		sceneLighting_.ambientColorR	= 0.2f;	 // Increased ambient for visibility
		sceneLighting_.ambientColorG	= 0.2f;
		sceneLighting_.ambientColorB	= 0.2f;
		sceneLighting_.ambientIntensity = 1.0f;

		// Main directional light
		sceneLighting_.lights[0].type		= LightType::Directional;
		sceneLighting_.lights[0].directionX = -0.5f;
		sceneLighting_.lights[0].directionY = -1.0f;
		sceneLighting_.lights[0].directionZ = -0.3f;
		sceneLighting_.lights[0].colorR		= 1.0f;
		sceneLighting_.lights[0].colorG		= 0.95f;
		sceneLighting_.lights[0].colorB		= 0.9f;
		sceneLighting_.lights[0].intensity	= 3.0f;

		// Initialize orbit camera
		orbitYaw_	   = 0.0f;
		orbitPitch_	   = 0.3f;	// Slight downward angle
		orbitDistance_ = 4.0f;
		lastMouseX_	   = 0.0;
		lastMouseY_	   = 0.0;
		isOrbiting_	   = false;

		GetSoul().Input().AddMousePositionCallback(
			[this](double x, double y)
			{
				if (isOrbiting_)
				{
					double deltaX = x - lastMouseX_;
					double deltaY = y - lastMouseY_;

					// Sensitivity
					const float sensitivity	 = 0.005f;
					orbitYaw_				-= static_cast<float>(deltaX) * sensitivity;
					orbitPitch_				+= static_cast<float>(deltaY) * sensitivity;

					// Clamp pitch to avoid gimbal lock
					const float maxPitch = 1.5f;  // ~85 degrees
					if (orbitPitch_ > maxPitch)
					{
						orbitPitch_ = maxPitch;
					}
					if (orbitPitch_ < -maxPitch)
					{
						orbitPitch_ = -maxPitch;
					}
				}
				lastMouseX_ = x;
				lastMouseY_ = y;
			});

		GetSoul().Input().AddMouseButtonCallback(
			[this](std::uint32_t button, ButtonState state)
			{
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

		UpdateGameLogic(current, previous);
		RenderFrame(current, previous);
	}

	void OnShutdown() override
	{
		// Destroy GPU mesh before raster backend
		if (meshUploader_.has_value() && cubeGPUMesh_.IsValid())
		{
			meshUploader_->DestroyMesh(cubeGPUMesh_);
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
	void UpdateGameLogic(Frame& current, Frame& previous)
	{
		// Update camera position based on orbit parameters
		float camX = orbitDistance_ * std::cos(orbitPitch_) * std::sin(orbitYaw_);
		float camY = orbitDistance_ * std::sin(orbitPitch_);
		float camZ = orbitDistance_ * std::cos(orbitPitch_) * std::cos(orbitYaw_);

		cameraPosition_ = {camX, camY, camZ};
		cameraTarget_	= {0.0f, 0.0f, 0.0f};

		// Update view matrix
		viewMatrix_ = synodic::math::LookAt(
			cameraPosition_,
			cameraTarget_,
			synodic::math::vec3{0.0f, 1.0f, 0.0f});

		// Update scene lighting camera position for specular
		sceneLighting_.cameraPositionX = cameraPosition_.x;
		sceneLighting_.cameraPositionY = cameraPosition_.y;
		sceneLighting_.cameraPositionZ = cameraPosition_.z;

		// Model matrix - static cube at origin
		modelMatrix_ = synodic::math::Mat4<float>::identity();

		// Compute MVP and push constant data
		synodic::math::mat4 vp = projectionMatrix_ * viewMatrix_;
		pushConstants_.mvp	 = vp * modelMatrix_;
		pushConstants_.model = modelMatrix_;
	}

	void RenderFrame(Frame& current, Frame& previous)
	{
		if (!hasValidSurface_)
		{
			return;
		}

		// Update material and scene lighting uniform buffers
		// Note: Cast to VulkanRasterBackend to access the update methods
		// In a more complete design, this would be part of the RasterModule interface
		auto& raster = GetSoul().Raster();
		raster.UpdateMaterial(&cubeMaterial_, sizeof(cubeMaterial_));
		raster.UpdateSceneLighting(&sceneLighting_, sizeof(sceneLighting_));

		auto& renderGraph = GetSoul().RenderGraph();

		// Begin a new frame - clears the previous graph
		renderGraph.BeginFrame();

		// Import the surface as an external resource
		ResourceHandle surfaceResource = renderGraph.ImportSurface(surfaceEntity_);

		// === Multi-Pass Rendering ===
		// Pass 1: Depth Pre-Pass - writes depth buffer only (no color)
		// Pass 2: Main PBR Pass - renders color with depth testing

		// Lambda to record cube draw commands
		auto recordCubeDraw = [this](PassExecutionContext& ctx)
		{
			if (!cubeGPUMesh_.IsValid())
			{
				return;
			}

			// Set push constants (MVP + Model matrices)
			ctx.commands.PushConstants(pushConstants_);

			// Draw the cube mesh (binds vertex/index buffers and issues DrawIndexed)
			ctx.commands.DrawMesh(
				cubeGPUMesh_.vertexBuffer,
				cubeGPUMesh_.indexBuffer,
				cubeGPUMesh_.indexCount,
				true);	// 32-bit indices
		};

		// === Depth Pre-Pass ===
		// Renders geometry to depth buffer only
		// Benefits: Early-Z rejection, reduces overdraw in main pass
		auto depthPrePass = RenderPassBuilder("DepthPrePass")
								.DepthOutput(surfaceResource, ResourceUsage::DepthStencilWrite)
								.ClearDepth(1.0f)  // Far plane (reverse-Z would use 0.0f)
								.Build();

		renderGraph.AddPassWithCallback(std::move(depthPrePass), recordCubeDraw);

		// === Main PBR Pass ===
		// Renders color with PBR shading, reads depth from pre-pass
		auto mainPass = RenderPassBuilder("MainPBRPass")
							.ColorOutput(surfaceResource, ResourceUsage::Present)
							.DepthInput(surfaceResource, ResourceUsage::DepthStencilRead)
							.ClearColor(0.1f, 0.1f, 0.15f, 1.0f)  // Dark blue-gray background
							.Build();

		renderGraph.AddPassWithCallback(std::move(mainPass), recordCubeDraw);

		// Execute the render graph with multi-pass support
		// This will:
		// 1. Compile the graph (topological sort, pass merging, aliasing)
		// 2. Allocate transient resources
		// 3. Insert barriers
		// 4. Execute depth pre-pass (FirstPass flag)
		// 5. Execute main PBR pass (LastPass flag)
		renderGraph.Execute(surfaceEntity_, surfaceSize_);

		// Present the frame
		renderGraph.Present();
	}

	// Window/surface state
	Entity surfaceEntity_;
	synodic::math::uvec2 surfaceSize_ = {0, 0};
	bool hasValidSurface_			  = false;

	// Mesh data (CPU-side, ready for GPU upload)
	MeshData cubeMeshData_;
	GPUMesh cubeGPUMesh_;  // GPU buffers for the cube mesh

	// Mesh uploader (deferred construction after Raster is ready)
	std::optional<MeshUploader> meshUploader_;

	// Material
	PBRMaterialData cubeMaterial_;

	// Scene lighting
	SceneLightingData sceneLighting_;

	// Camera
	synodic::math::vec3 cameraPosition_;
	synodic::math::vec3 cameraTarget_;

	// Orbit camera controls
	float orbitYaw_		 = 0.0f;
	float orbitPitch_	 = 0.3f;
	float orbitDistance_ = 4.0f;
	double lastMouseX_	 = 0.0;
	double lastMouseY_	 = 0.0;
	bool isOrbiting_	 = false;

	// Transforms
	synodic::math::mat4 modelMatrix_;
	synodic::math::mat4 viewMatrix_;
	synodic::math::mat4 projectionMatrix_;
	PushConstantData pushConstants_;
};

std::int32_t main(std::int32_t, char*[])
{
	const synodic::soul::Parameters appParams;
	SDLBackend backend;
	SDLWindowBackend windowBackend(backend);
	SDLInputBackend inputBackend(backend, windowBackend);

	Property<std::uint32_t> threadCount(1);
	PassthroughSchedulerBackend schedulerBackend(threadCount);
	Sample app(appParams, std::move(inputBackend), std::move(windowBackend), schedulerBackend);

	app.Run();

	return 0;
}
