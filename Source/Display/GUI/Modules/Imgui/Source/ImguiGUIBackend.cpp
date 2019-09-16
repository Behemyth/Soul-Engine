#include "ImguiBackend.h"

#include "WindowParameters.h"
#include "Display/Window/WindowModule.h"
#include "Display/Window/Window.h"
#include "Display/Input/InputModule.h"
#include "Render/RenderGraph/RenderGraphModule.h"

#define IMGUI_USER_CONFIG "ImguiConfig.h"
#include <imgui.h>

struct PushBlock {

	glm::vec2 scale;
	glm::vec2 translate;

} pushBlock;

ImguiBackend::ImguiBackend(std::shared_ptr<InputModule>& inputModule,
	std::shared_ptr<WindowModule>& windowModule,
	std::shared_ptr<RenderGraphModule>& renderGraphModule):
	inputModule_(inputModule),
	windowModule_(windowModule), renderGraphModule_(renderGraphModule)
{

	ImGui::CreateContext();

	// Set callbacks
	inputModule_->AddMousePositionCallback([](double xPos, double yPos) {
		ImGuiIO& inputInfo = ImGui::GetIO();

		inputInfo.MousePos = ImVec2(static_cast<float>(xPos), static_cast<float>(yPos));
	});

	inputModule_->AddMouseButtonCallback([](int button, ButtonState state) {
		if (button > 1) {
			return;
		}

		ImGuiIO& inputInfo = ImGui::GetIO();

		inputInfo.MouseDown[button] = state == ButtonState::PRESS;
	});


	ImGuiIO& inputInfo = ImGui::GetIO();

	// TODO: abstract fonts
	// Grab and upload font data
	unsigned char* fontData;
	int textureWidth, textureHeight;
	inputInfo.Fonts->GetTexDataAsRGBA32(&fontData, &textureWidth, &textureHeight);

	// TODO: Create font buffers

	RenderTaskParameters params;
	params.name = "GUI";

	renderGraphModule_->CreateRenderPass(params, [](RenderGraphBuilder& builder) {
		Entity vertexBufferResource = builder.Request<VertexBuffer>();
		Entity indexBufferResource = builder.Request<IndexBuffer>();
		Entity pushBufferResource = builder.Request<PushBuffer>();
		Entity renderViewResource = builder.View();

		return [=](const EntityRegistry& registry, CommandList& commandList) {
			auto& renderView = registry.GetComponent<RenderView>(renderViewResource);
			auto& pushBuffer = registry.GetComponent<PushBuffer>(pushBufferResource);

			// Input
			{

				ImGuiIO& io = ImGui::GetIO();
				renderView.width = io.DisplaySize.x;
				renderView.height = io.DisplaySize.y;
				renderView.minDepth = 0.0f;
				renderView.maxDepth = 1.0f;

				pushBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
				pushBlock.translate = glm::vec2(-1.0f);

				{
					UpdateBufferCommand updatePushParameters;
					updatePushParameters.buffer = pushBufferResource;
					updatePushParameters.data =
						nonstd::as_writeable_bytes(nonstd::span(&pushBlock, 1));
					updatePushParameters.offset = 0;

					commandList.UpdateBuffer(updatePushParameters);
				}
			}

			ImDrawData* drawData = ImGui::GetDrawData();

			if (drawData->TotalVtxCount == 0 || drawData->TotalIdxCount == 0) {

				return;
			}

			auto& vertexBuffer = registry.GetComponent<VertexBuffer>(vertexBufferResource);
			auto& indexBuffer = registry.GetComponent<IndexBuffer>(indexBufferResource);

			// Push the data to the buffers
			{
				auto vertexOffset = 0;
				auto indexOffset = 0;

				for (int32 i = 0; i < drawData->CmdListsCount; i++) {

					const ImDrawList* imguiCommand = drawData->CmdLists[i];

					{

						UpdateBufferCommand updateVertexParameters;
						updateVertexParameters.buffer = vertexBufferResource;
						updateVertexParameters.data = nonstd::as_writeable_bytes(nonstd::span(
							imguiCommand->VtxBuffer.Data, imguiCommand->VtxBuffer.Size));
						updateVertexParameters.offset = vertexOffset;
						commandList.UpdateBuffer(updateVertexParameters);
					}

					{

						UpdateBufferCommand updateIndexParameters;
						updateIndexParameters.buffer = indexBufferResource;

						updateIndexParameters.data = nonstd::as_writeable_bytes(nonstd::span(
							imguiCommand->IdxBuffer.Data, imguiCommand->IdxBuffer.Size));
						updateIndexParameters.offset = indexOffset;
						commandList.UpdateBuffer(updateIndexParameters);
					}

					vertexOffset += imguiCommand->VtxBuffer.Size;
					indexOffset += imguiCommand->IdxBuffer.Size;
				}
			}

			// Drawing
			{

				int vertexOffset = 0;
				int indexOffset = 0;

				for (int32 i = 0; i < drawData->CmdListsCount; i++) {

					const ImDrawList* imguiCommands = drawData->CmdLists[i];

					for (int32 j = 0; j < imguiCommands->CmdBuffer.Size; j++) {

						const ImDrawCmd* command = &imguiCommands->CmdBuffer[j];

						{
							DrawCommand drawParameters;
							drawParameters.elementSize = command->ElemCount;
							drawParameters.indexOffset = indexOffset;
							drawParameters.vertexOffset = vertexOffset;
							drawParameters.scissorOffset = {
								command->ClipRect.x, command->ClipRect.y};
							drawParameters.scissorExtent = {
								command->ClipRect.z - command->ClipRect.x,
								command->ClipRect.w - command->ClipRect.y};

							drawParameters.vertexBuffer = vertexBufferResource;
							drawParameters.indexBuffer = indexBufferResource;

							commandList.Draw(drawParameters);
						}

						indexOffset += command->ElemCount;
					}

					vertexOffset += imguiCommands->VtxBuffer.Size;
				}
			}
		};
	});
}

ImguiBackend::~ImguiBackend()
{

	ImGui::DestroyContext();
}

void ImguiBackend::Update(std::chrono::nanoseconds frameTime)
{

	ImGuiIO& inputInfo = ImGui::GetIO();

	// TODO: use the GUI associated window
	// TODO: via callback
	// Update Display
	WindowParameters& windowParams = windowModule_->MasterWindow().Parameters();
	inputInfo.DisplaySize = ImVec2(
		static_cast<float>(windowParams.pixelSize.x), static_cast<float>(windowParams.pixelSize.y));
	inputInfo.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	// TODO: via callback
	// Update frame timings
	auto frameSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(frameTime);
	inputInfo.DeltaTime = frameSeconds.count();


	ImGui::NewFrame();
	ConvertRetained();
	ImGui::Render();
}

void ImguiBackend::ConvertRetained()
{

	// TODO: Convert retained framework to dear imgui intermediate
	// TODO: Remove hardcoded gui

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Exit")) {

				// TODO: Call an exit command

				ImGui::EndMenu();
			}
		}

		ImGui::EndMainMenuBar();
	}
}