#pragma once

#include "Render/Raster/RenderCommands.h"

#include "Core/Utility/Thread/ThreadLocal.h"

#include <memory>
#include <list>
#include <vector>

class CommandList {

public:

	CommandList();
	~CommandList() = default;

	// Agnostic raster API interface
	void Draw(DrawCommand&);
	void DrawIndirect(DrawIndirectCommand&);
	void UpdateBuffer(UpdateBufferCommand&);
	void UpdateTexture(UpdateTextureCommand&);
	void CopyBuffer(CopyBufferCommand&);
	void CopyTexture(CopyTextureCommand&);

private:

	std::list<std::pair<CommandType, uint>> commands_;

	std::vector<DrawCommand> drawCommands_;
	std::vector<DrawIndirectCommand> drawIndirectCommands_;
	std::vector<UpdateBufferCommand> updateBufferCommands_;
	std::vector<UpdateTextureCommand> updateTextureCommands_;
	std::vector<CopyBufferCommand> copyBufferCommands_;
	std::vector<CopyTextureCommand> copyTextureCommands_;



};