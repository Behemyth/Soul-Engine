export module synodic.soul.raster:command_list;

import std;
import :commands;

export class CommandList {

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

	std::list<std::pair<CommandType, std::uint32_t>> commands_;

	std::vector<DrawCommand> drawCommands_;
	std::vector<DrawIndirectCommand> drawIndirectCommands_;
	std::vector<UpdateBufferCommand> updateBufferCommands_;
	std::vector<UpdateTextureCommand> updateTextureCommands_;
	std::vector<CopyBufferCommand> copyBufferCommands_;
	std::vector<CopyTextureCommand> copyTextureCommands_;



};

inline CommandList::CommandList(): commands_(5)
{
}

inline void CommandList::Draw(DrawCommand& command)
{

	auto pos = drawCommands_.size();
	drawCommands_.push_back(command);
	commands_.push_back({CommandType::Draw, pos});
}

inline void CommandList::DrawIndirect(DrawIndirectCommand& command)
{
	auto pos = drawIndirectCommands_.size();
	drawIndirectCommands_.push_back(command);
	commands_.push_back({CommandType::DrawIndirect, pos});
}

inline void CommandList::UpdateBuffer(UpdateBufferCommand& command)
{
	auto pos = updateBufferCommands_.size();
	updateBufferCommands_.push_back(command);
	commands_.push_back({CommandType::UpdateBuffer, pos});
}

inline void CommandList::UpdateTexture(UpdateTextureCommand& command)
{
	auto pos = updateTextureCommands_.size();
	updateTextureCommands_.push_back(command);
	commands_.push_back({CommandType::UpdateTexture, pos});
}

inline void CommandList::CopyBuffer(CopyBufferCommand& command)
{
	auto pos = copyBufferCommands_.size();
	copyBufferCommands_.push_back(command);
	commands_.push_back({CommandType::CopyBuffer, pos});
}

inline void CommandList::CopyTexture(CopyTextureCommand& command)
{
	auto pos = copyTextureCommands_.size();
	copyTextureCommands_.push_back(command);
	commands_.push_back({CommandType::CopyTexture, pos});
}
