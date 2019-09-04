#include "CommandList.h"
#include "Core/Utility/Exception/Exception.h"

#include "Render/Raster/RasterModule.h"

CommandList::CommandList(): commands_(5)
{
}

void CommandList::Draw(DrawCommand& command)
{

	auto pos = drawCommands_.size();
	drawCommands_.push_back(command);
	commands_.push_back({CommandType::Draw, pos});
}

void CommandList::DrawIndirect(DrawIndirectCommand& command)
{
	auto pos = drawIndirectCommands_.size();
	drawIndirectCommands_.push_back(command);
	commands_.push_back({CommandType::DrawIndirect, pos});
}

void CommandList::UpdateBuffer(UpdateBufferCommand& command)
{
	auto pos = updateBufferCommands_.size();
	updateBufferCommands_.push_back(command);
	commands_.push_back({CommandType::UpdateBuffer, pos});
}

void CommandList::UpdateTexture(UpdateTextureCommand& command)
{
	auto pos = updateTextureCommands_.size();
	updateTextureCommands_.push_back(command);
	commands_.push_back({CommandType::UpdateTexture, pos});
}

void CommandList::CopyBuffer(CopyBufferCommand& command)
{
	auto pos = copyBufferCommands_.size();
	copyBufferCommands_.push_back(command);
	commands_.push_back({CommandType::CopyBuffer, pos});
}

void CommandList::CopyTexture(CopyTextureCommand& command)
{
	auto pos = copyTextureCommands_.size();
	copyTextureCommands_.push_back(command);
	commands_.push_back({CommandType::CopyTexture, pos});
}