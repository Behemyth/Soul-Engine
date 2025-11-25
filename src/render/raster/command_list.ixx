export module synodic.soul.raster:command_list;

import std;
import :commands;

// Command entry storing type and index into type-specific storage
struct CommandEntry {
	CommandType type;
	std::uint32_t index;
};

export class CommandList {

public:

	CommandList() = default;
	~CommandList() = default;

	// Clear all commands for reuse
	void Clear() {
		commands_.clear();
		drawCommands_.clear();
		drawIndexedCommands_.clear();
		drawIndirectCommands_.clear();
		bindVertexBufferCommands_.clear();
		bindIndexBufferCommands_.clear();
		setPushConstantsCommands_.clear();
		bindPipelineCommands_.clear();
		setViewportCommands_.clear();
		setScissorCommands_.clear();
		updateBufferCommands_.clear();
		updateTextureCommands_.clear();
		copyBufferCommands_.clear();
		copyTextureCommands_.clear();
	}

	// Check if command list is empty
	[[nodiscard]] bool Empty() const { return commands_.empty(); }
	
	// Get number of commands
	[[nodiscard]] std::size_t Size() const { return commands_.size(); }

	// === Command Recording API ===
	
	void Draw(const DrawCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawCommands_.size());
		drawCommands_.push_back(command);
		commands_.push_back({CommandType::Draw, pos});
	}
	
	void DrawIndexed(const DrawIndexedCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawIndexedCommands_.size());
		drawIndexedCommands_.push_back(command);
		commands_.push_back({CommandType::DrawIndexed, pos});
	}
	
	void DrawIndirect(const DrawIndirectCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawIndirectCommands_.size());
		drawIndirectCommands_.push_back(command);
		commands_.push_back({CommandType::DrawIndirect, pos});
	}
	
	void BindVertexBuffer(const BindVertexBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(bindVertexBufferCommands_.size());
		bindVertexBufferCommands_.push_back(command);
		commands_.push_back({CommandType::BindVertexBuffer, pos});
	}
	
	void BindIndexBuffer(const BindIndexBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(bindIndexBufferCommands_.size());
		bindIndexBufferCommands_.push_back(command);
		commands_.push_back({CommandType::BindIndexBuffer, pos});
	}
	
	void SetPushConstants(const SetPushConstantsCommand& command) {
		auto pos = static_cast<std::uint32_t>(setPushConstantsCommands_.size());
		setPushConstantsCommands_.push_back(command);
		commands_.push_back({CommandType::SetPushConstants, pos});
	}
	
	void BindPipeline(const BindPipelineCommand& command) {
		auto pos = static_cast<std::uint32_t>(bindPipelineCommands_.size());
		bindPipelineCommands_.push_back(command);
		commands_.push_back({CommandType::BindPipeline, pos});
	}
	
	void SetViewport(const SetViewportCommand& command) {
		auto pos = static_cast<std::uint32_t>(setViewportCommands_.size());
		setViewportCommands_.push_back(command);
		commands_.push_back({CommandType::SetViewport, pos});
	}
	
	void SetScissor(const SetScissorCommand& command) {
		auto pos = static_cast<std::uint32_t>(setScissorCommands_.size());
		setScissorCommands_.push_back(command);
		commands_.push_back({CommandType::SetScissor, pos});
	}
	
	void UpdateBuffer(const UpdateBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(updateBufferCommands_.size());
		updateBufferCommands_.push_back(command);
		commands_.push_back({CommandType::UpdateBuffer, pos});
	}
	
	void UpdateTexture(const UpdateTextureCommand& command) {
		auto pos = static_cast<std::uint32_t>(updateTextureCommands_.size());
		updateTextureCommands_.push_back(command);
		commands_.push_back({CommandType::UpdateTexture, pos});
	}
	
	void CopyBuffer(const CopyBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(copyBufferCommands_.size());
		copyBufferCommands_.push_back(command);
		commands_.push_back({CommandType::CopyBuffer, pos});
	}
	
	void CopyTexture(const CopyTextureCommand& command) {
		auto pos = static_cast<std::uint32_t>(copyTextureCommands_.size());
		copyTextureCommands_.push_back(command);
		commands_.push_back({CommandType::CopyTexture, pos});
	}
	
	// === Convenience helpers ===
	
	// Record a simple indexed draw with buffer bindings
	void DrawMesh(GPUBufferHandle vertexBuffer, GPUBufferHandle indexBuffer, 
	              std::uint32_t indexCount, bool use32BitIndices = true) {
		BindVertexBuffer({vertexBuffer, 0, 0});
		BindIndexBuffer({indexBuffer, 0, use32BitIndices});
		DrawIndexed({indexCount, 1, 0, 0, 0});
	}
	
	// Record push constants from typed data
	template<typename T>
	void PushConstants(const T& data) {
		SetPushConstantsCommand cmd;
		cmd.Set(data);
		SetPushConstants(cmd);
	}

	// === Iteration API for backend consumption ===
	
	// Visitor pattern for type-safe command processing
	template<typename Visitor>
	void ForEach(Visitor&& visitor) const {
		for (const auto& entry : commands_) {
			switch (entry.type) {
				case CommandType::Draw:
					visitor(drawCommands_[entry.index]);
					break;
				case CommandType::DrawIndexed:
					visitor(drawIndexedCommands_[entry.index]);
					break;
				case CommandType::DrawIndirect:
					visitor(drawIndirectCommands_[entry.index]);
					break;
				case CommandType::BindVertexBuffer:
					visitor(bindVertexBufferCommands_[entry.index]);
					break;
				case CommandType::BindIndexBuffer:
					visitor(bindIndexBufferCommands_[entry.index]);
					break;
				case CommandType::SetPushConstants:
					visitor(setPushConstantsCommands_[entry.index]);
					break;
				case CommandType::BindPipeline:
					visitor(bindPipelineCommands_[entry.index]);
					break;
				case CommandType::SetViewport:
					visitor(setViewportCommands_[entry.index]);
					break;
				case CommandType::SetScissor:
					visitor(setScissorCommands_[entry.index]);
					break;
				case CommandType::UpdateBuffer:
					visitor(updateBufferCommands_[entry.index]);
					break;
				case CommandType::UpdateTexture:
					visitor(updateTextureCommands_[entry.index]);
					break;
				case CommandType::CopyBuffer:
					visitor(copyBufferCommands_[entry.index]);
					break;
				case CommandType::CopyTexture:
					visitor(copyTextureCommands_[entry.index]);
					break;
			}
		}
	}
	
	// Index-based iteration for backends that need command indices
	[[nodiscard]] std::size_t CommandCount() const { return commands_.size(); }
	
	[[nodiscard]] CommandType GetCommandType(std::size_t index) const {
		return commands_[index].type;
	}
	
	// Type-specific accessors
	[[nodiscard]] const DrawCommand& GetDraw(std::size_t cmdIndex) const {
		return drawCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const DrawIndexedCommand& GetDrawIndexed(std::size_t cmdIndex) const {
		return drawIndexedCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const BindVertexBufferCommand& GetBindVertexBuffer(std::size_t cmdIndex) const {
		return bindVertexBufferCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const BindIndexBufferCommand& GetBindIndexBuffer(std::size_t cmdIndex) const {
		return bindIndexBufferCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const SetPushConstantsCommand& GetSetPushConstants(std::size_t cmdIndex) const {
		return setPushConstantsCommands_[commands_[cmdIndex].index];
	}

private:

	std::vector<CommandEntry> commands_;

	std::vector<DrawCommand> drawCommands_;
	std::vector<DrawIndexedCommand> drawIndexedCommands_;
	std::vector<DrawIndirectCommand> drawIndirectCommands_;
	std::vector<BindVertexBufferCommand> bindVertexBufferCommands_;
	std::vector<BindIndexBufferCommand> bindIndexBufferCommands_;
	std::vector<SetPushConstantsCommand> setPushConstantsCommands_;
	std::vector<BindPipelineCommand> bindPipelineCommands_;
	std::vector<SetViewportCommand> setViewportCommands_;
	std::vector<SetScissorCommand> setScissorCommands_;
	std::vector<UpdateBufferCommand> updateBufferCommands_;
	std::vector<UpdateTextureCommand> updateTextureCommands_;
	std::vector<CopyBufferCommand> copyBufferCommands_;
	std::vector<CopyTextureCommand> copyTextureCommands_;

};
