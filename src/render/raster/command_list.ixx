/**
 * @file command_list.ixx
 * @brief Command list for recording GPU commands using bindless patterns
 * 
 * All commands follow "No Graphics API" patterns:
 * - GPU pointer root arguments for shader data
 * - Stage-only barriers
 * - Dynamic depth/stencil/blend state
 * - No legacy binding commands (vertex buffers, index buffers, push constants)
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:command_list;

import std;
import :commands;
import :depth_stencil_state;
import :blend_state;

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
		// Draw commands
		drawCommands_.clear();
		drawIndexedCommands_.clear();
		drawWithPointersCommands_.clear();
		drawIndirectWithPointersCommands_.clear();
		// Compute commands
		dispatchCommands_.clear();
		dispatchIndirectCommands_.clear();
		// State commands
		bindPipelineCommands_.clear();
		setViewportCommands_.clear();
		setScissorCommands_.clear();
		setDepthStencilStateCommands_.clear();
		setBlendStateCommands_.clear();
		setTextureHeapCommands_.clear();
		// Synchronization
		barrierCommands_.clear();
		memoryBarrierCommands_.clear();
		// Resource operations
		updateBufferCommands_.clear();
		copyBufferCommands_.clear();
	}

	// Check if command list is empty
	[[nodiscard]] bool Empty() const { return commands_.empty(); }
	
	// Get number of commands
	[[nodiscard]] std::size_t Size() const { return commands_.size(); }

	// ========================================================================
	// Draw Commands
	// ========================================================================
	
	/**
	 * @brief Record a basic non-indexed draw
	 * 
	 * For simple draws that don't need GPU pointer root arguments.
	 * Uses currently bound pipeline's vertex input.
	 */
	void Draw(const DrawCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawCommands_.size());
		drawCommands_.push_back(command);
		commands_.push_back({CommandType::Draw, pos});
	}
	
	/**
	 * @brief Record a basic indexed draw
	 * 
	 * For simple indexed draws. Requires prior index buffer bind via
	 * DrawWithPointers pattern for bindless systems.
	 */
	void DrawIndexed(const DrawIndexedCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawIndexedCommands_.size());
		drawIndexedCommands_.push_back(command);
		commands_.push_back({CommandType::DrawIndexed, pos});
	}
	
	/**
	 * @brief Record a draw with GPU pointer root arguments (bindless pattern)
	 * 
	 * Primary draw command for bindless rendering. Passes GPU pointers
	 * to vertex and pixel shader data structs.
	 */
	void DrawWithPointers(const DrawWithPointersCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawWithPointersCommands_.size());
		drawWithPointersCommands_.push_back(command);
		commands_.push_back({CommandType::DrawWithPointers, pos});
	}
	
	/**
	 * @brief Record a multi-draw indirect with GPU pointers (fully GPU-driven)
	 * 
	 * Ultimate bindless pattern for culling/LOD systems where both
	 * draw arguments AND per-draw data pointers are GPU-generated.
	 */
	void DrawIndirectWithPointers(const DrawIndirectWithPointersCommand& command) {
		auto pos = static_cast<std::uint32_t>(drawIndirectWithPointersCommands_.size());
		drawIndirectWithPointersCommands_.push_back(command);
		commands_.push_back({CommandType::DrawIndirectWithPointers, pos});
	}
	
	// ========================================================================
	// Compute Commands
	// ========================================================================
	
	/**
	 * @brief Dispatch compute shader with GPU pointer root data
	 */
	void Dispatch(const DispatchCommand& command) {
		auto pos = static_cast<std::uint32_t>(dispatchCommands_.size());
		dispatchCommands_.push_back(command);
		commands_.push_back({CommandType::Dispatch, pos});
	}
	
	/**
	 * @brief Dispatch compute shader with GPU-generated arguments
	 */
	void DispatchIndirect(const DispatchIndirectCommand& command) {
		auto pos = static_cast<std::uint32_t>(dispatchIndirectCommands_.size());
		dispatchIndirectCommands_.push_back(command);
		commands_.push_back({CommandType::DispatchIndirect, pos});
	}
	
	// ========================================================================
	// State Commands
	// ========================================================================
	
	/**
	 * @brief Bind a pipeline for subsequent draws/dispatches
	 */
	void BindPipeline(const BindPipelineCommand& command) {
		auto pos = static_cast<std::uint32_t>(bindPipelineCommands_.size());
		bindPipelineCommands_.push_back(command);
		commands_.push_back({CommandType::BindPipeline, pos});
	}
	
	/**
	 * @brief Set viewport dynamically
	 */
	void SetViewport(const SetViewportCommand& command) {
		auto pos = static_cast<std::uint32_t>(setViewportCommands_.size());
		setViewportCommands_.push_back(command);
		commands_.push_back({CommandType::SetViewport, pos});
	}
	
	/**
	 * @brief Set scissor rect dynamically
	 */
	void SetScissor(const SetScissorCommand& command) {
		auto pos = static_cast<std::uint32_t>(setScissorCommands_.size());
		setScissorCommands_.push_back(command);
		commands_.push_back({CommandType::SetScissor, pos});
	}
	
	/**
	 * @brief Set depth-stencil state dynamically (VK_EXT_extended_dynamic_state)
	 */
	void SetDepthStencilState(const SetDepthStencilStateCommand& command) {
		auto pos = static_cast<std::uint32_t>(setDepthStencilStateCommands_.size());
		setDepthStencilStateCommands_.push_back(command);
		commands_.push_back({CommandType::SetDepthStencilState, pos});
	}
	
	// Convenience: Set depth-stencil state from state object directly
	void SetDepthStencilState(const DepthStencilState& state) {
		SetDepthStencilState(SetDepthStencilStateCommand{state});
	}
	
	/**
	 * @brief Set blend state dynamically (VK_EXT_extended_dynamic_state3)
	 */
	void SetBlendState(const SetBlendStateCommand& command) {
		auto pos = static_cast<std::uint32_t>(setBlendStateCommands_.size());
		setBlendStateCommands_.push_back(command);
		commands_.push_back({CommandType::SetBlendState, pos});
	}
	
	// Convenience: Set blend state from state object directly
	void SetBlendState(const BlendState& state) {
		SetBlendState(SetBlendStateCommand{state});
	}
	
	/**
	 * @brief Set active texture/sampler heaps for bindless access
	 */
	void SetTextureHeap(const SetTextureHeapCommand& command) {
		auto pos = static_cast<std::uint32_t>(setTextureHeapCommands_.size());
		setTextureHeapCommands_.push_back(command);
		commands_.push_back({CommandType::SetTextureHeap, pos});
	}
	
	// ========================================================================
	// Synchronization Commands
	// ========================================================================
	
	/**
	 * @brief Insert stage-only pipeline barrier
	 * 
	 * Follows "No Graphics API" pattern of simplified barriers without
	 * per-resource tracking - just sync between pipeline stages.
	 */
	void Barrier(const BarrierCommand& command) {
		auto pos = static_cast<std::uint32_t>(barrierCommands_.size());
		barrierCommands_.push_back(command);
		commands_.push_back({CommandType::Barrier, pos});
	}
	
	// Convenience: Create barrier between stages with hazard type
	void Barrier(StageFlags srcStage, StageFlags dstStage, HazardFlags hazard) {
		Barrier(BarrierCommand::Between(srcStage, dstStage, hazard));
	}
	
	/**
	 * @brief Insert a global memory barrier
	 */
	void MemoryBarrier(const MemoryBarrierCommand& command) {
		auto pos = static_cast<std::uint32_t>(memoryBarrierCommands_.size());
		memoryBarrierCommands_.push_back(command);
		commands_.push_back({CommandType::MemoryBarrier, pos});
	}
	
	// ========================================================================
	// Resource Operations
	// ========================================================================
	
	/**
	 * @brief Update buffer contents from CPU data
	 */
	void UpdateBuffer(const UpdateBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(updateBufferCommands_.size());
		updateBufferCommands_.push_back(command);
		commands_.push_back({CommandType::UpdateBuffer, pos});
	}
	
	/**
	 * @brief Copy between GPU buffers
	 */
	void CopyBuffer(const CopyBufferCommand& command) {
		auto pos = static_cast<std::uint32_t>(copyBufferCommands_.size());
		copyBufferCommands_.push_back(command);
		commands_.push_back({CommandType::CopyBuffer, pos});
	}

	// ========================================================================
	// Iteration API for Backend Consumption
	// ========================================================================
	
	/**
	 * @brief Visitor pattern for type-safe command processing
	 * 
	 * Visitor must be callable with all command types.
	 */
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
				case CommandType::DrawWithPointers:
					visitor(drawWithPointersCommands_[entry.index]);
					break;
				case CommandType::DrawIndirectWithPointers:
					visitor(drawIndirectWithPointersCommands_[entry.index]);
					break;
				case CommandType::Dispatch:
					visitor(dispatchCommands_[entry.index]);
					break;
				case CommandType::DispatchIndirect:
					visitor(dispatchIndirectCommands_[entry.index]);
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
				case CommandType::SetDepthStencilState:
					visitor(setDepthStencilStateCommands_[entry.index]);
					break;
				case CommandType::SetBlendState:
					visitor(setBlendStateCommands_[entry.index]);
					break;
				case CommandType::SetTextureHeap:
					visitor(setTextureHeapCommands_[entry.index]);
					break;
				case CommandType::Barrier:
					visitor(barrierCommands_[entry.index]);
					break;
				case CommandType::MemoryBarrier:
					visitor(memoryBarrierCommands_[entry.index]);
					break;
				case CommandType::UpdateBuffer:
					visitor(updateBufferCommands_[entry.index]);
					break;
				case CommandType::CopyBuffer:
					visitor(copyBufferCommands_[entry.index]);
					break;
			}
		}
	}
	
	// Index-based iteration for backends that need command indices
	[[nodiscard]] std::size_t CommandCount() const { return commands_.size(); }
	
	[[nodiscard]] CommandType GetCommandType(std::size_t index) const {
		return commands_[index].type;
	}
	
	// Type-specific accessors for backends that need random access
	[[nodiscard]] const DrawCommand& GetDraw(std::size_t cmdIndex) const {
		return drawCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const DrawIndexedCommand& GetDrawIndexed(std::size_t cmdIndex) const {
		return drawIndexedCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const DrawWithPointersCommand& GetDrawWithPointers(std::size_t cmdIndex) const {
		return drawWithPointersCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const DispatchCommand& GetDispatch(std::size_t cmdIndex) const {
		return dispatchCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const SetDepthStencilStateCommand& GetSetDepthStencilState(std::size_t cmdIndex) const {
		return setDepthStencilStateCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const SetBlendStateCommand& GetSetBlendState(std::size_t cmdIndex) const {
		return setBlendStateCommands_[commands_[cmdIndex].index];
	}
	
	[[nodiscard]] const BarrierCommand& GetBarrier(std::size_t cmdIndex) const {
		return barrierCommands_[commands_[cmdIndex].index];
	}

private:

	std::vector<CommandEntry> commands_;

	// Draw commands
	std::vector<DrawCommand> drawCommands_;
	std::vector<DrawIndexedCommand> drawIndexedCommands_;
	std::vector<DrawWithPointersCommand> drawWithPointersCommands_;
	std::vector<DrawIndirectWithPointersCommand> drawIndirectWithPointersCommands_;
	
	// Compute commands
	std::vector<DispatchCommand> dispatchCommands_;
	std::vector<DispatchIndirectCommand> dispatchIndirectCommands_;
	
	// State commands
	std::vector<BindPipelineCommand> bindPipelineCommands_;
	std::vector<SetViewportCommand> setViewportCommands_;
	std::vector<SetScissorCommand> setScissorCommands_;
	std::vector<SetDepthStencilStateCommand> setDepthStencilStateCommands_;
	std::vector<SetBlendStateCommand> setBlendStateCommands_;
	std::vector<SetTextureHeapCommand> setTextureHeapCommands_;
	
	// Synchronization
	std::vector<BarrierCommand> barrierCommands_;
	std::vector<MemoryBarrierCommand> memoryBarrierCommands_;
	
	// Resource operations
	std::vector<UpdateBufferCommand> updateBufferCommands_;
	std::vector<CopyBufferCommand> copyBufferCommands_;

};
