export module synodic.soul.render.graph;

export import :parameter;
export import :builder;
export import :pass;
export import :resource;

import synodic.soul.scheduler;
import synodic.soul.core;
import synodic.soul.raster;
import synodic.soul.graph;
import :parameter;
import :builder;
import :pass;
import :resource;
import std;

// NOTE: Using runtime AdjacencyListStorage for now.
// Plan: Switch to StaticStorage for compile-time graphs when #embed is available.
// The current implementation supports runtime modification for future editor bindings.

// Dependency edge in the render graph
export struct RenderGraphEdge {
	std::uint32_t sourcePass = 0;
	std::uint32_t destPass = 0;
	ResourceHandle resource = InvalidResourceHandle;  // Resource creating the dependency
	ResourceUsage sourceUsage = ResourceUsage::None;
	ResourceUsage destUsage = ResourceUsage::None;
};

// Compiled graph data - result of graph analysis
export struct CompiledRenderGraph {
	// Execution order (topologically sorted)
	std::vector<std::uint32_t> executionOrder;

	// Merged pass groups (for Vulkan subpass merging)
	std::vector<MergedPassGroup> mergedGroups;

	// Aliasing assignments
	std::vector<AliasingGroup> aliasingGroups;

	// Barriers needed between passes
	struct BarrierInfo {
		std::uint32_t afterPass = 0;
		std::uint32_t beforePass = 0;
		ResourceHandle resource = InvalidResourceHandle;
		ResourceUsage fromUsage = ResourceUsage::None;
		ResourceUsage toUsage = ResourceUsage::None;
	};
	std::vector<BarrierInfo> barriers;

	bool isValid = false;
};

// Main render graph class
export class RenderGraph {
public:
	RenderGraph() = default;
	~RenderGraph() = default;

	RenderGraph(const RenderGraph&) = delete;
	RenderGraph(RenderGraph&&) noexcept = default;

	RenderGraph& operator=(const RenderGraph&) = delete;
	RenderGraph& operator=(RenderGraph&&) noexcept = default;

	// Resource creation
	[[nodiscard]] ResourceHandle CreateTransientImage(const TransientImageDesc& desc) {
		return resources_.CreateTransientImage(desc);
	}

	[[nodiscard]] ResourceHandle CreateTransientBuffer(const TransientBufferDesc& desc) {
		return resources_.CreateTransientBuffer(desc);
	}

	[[nodiscard]] ResourceHandle ImportExternal(Entity entity, bool isImage = true, Format format = Format::RGBA) {
		return resources_.ImportExternal(entity, isImage, format);
	}

	// Pass creation
	[[nodiscard]] std::uint32_t AddRenderPass(RenderPassDesc&& desc) {
		const std::uint32_t index = static_cast<std::uint32_t>(renderPasses_.size());
		desc.originalIndex = index;
		renderPasses_.push_back(std::move(desc));
		dirty_ = true;
		return index;
	}

	[[nodiscard]] std::uint32_t AddComputePass(ComputePassDesc&& desc) {
		const std::uint32_t index = static_cast<std::uint32_t>(computePasses_.size());
		desc.originalIndex = index;
		computePasses_.push_back(std::move(desc));
		dirty_ = true;
		return index;
	}

	[[nodiscard]] std::uint32_t AddTransferPass(TransferPassDesc&& desc) {
		const std::uint32_t index = static_cast<std::uint32_t>(transferPasses_.size());
		desc.originalIndex = index;
		transferPasses_.push_back(std::move(desc));
		dirty_ = true;
		return index;
	}

	// Get pass descriptors for modification (runtime editor support)
	[[nodiscard]] RenderPassDesc* GetRenderPass(std::uint32_t index) {
		if (index < renderPasses_.size()) {
			dirty_ = true;
			return &renderPasses_[index];
		}
		return nullptr;
	}

	[[nodiscard]] ComputePassDesc* GetComputePass(std::uint32_t index) {
		if (index < computePasses_.size()) {
			dirty_ = true;
			return &computePasses_[index];
		}
		return nullptr;
	}

	// Compile the graph - builds execution order, merges passes, computes aliasing
	[[nodiscard]] const CompiledRenderGraph& Compile() {
		if (!dirty_ && compiled_.isValid) {
			return compiled_;
		}

		compiled_ = CompiledRenderGraph{};

		// Build dependency edges
		BuildDependencies();

		// Topological sort for execution order
		if (!TopologicalSort()) {
			return compiled_;  // Cycle detected
		}

		// Analyze for pass merging (Vulkan subpasses)
		MergePasses();

		// Compute resource lifetimes and aliasing
		ComputeAliasing();

		// Determine barrier placement
		ComputeBarriers();

		compiled_.isValid = true;
		dirty_ = false;

		return compiled_;
	}

	// Accessors
	[[nodiscard]] const std::vector<RenderPassDesc>& RenderPasses() const { return renderPasses_; }
	[[nodiscard]] const std::vector<ComputePassDesc>& ComputePasses() const { return computePasses_; }
	[[nodiscard]] const std::vector<TransferPassDesc>& TransferPasses() const { return transferPasses_; }
	[[nodiscard]] ResourceRegistry& Resources() { return resources_; }
	[[nodiscard]] const ResourceRegistry& Resources() const { return resources_; }
	[[nodiscard]] bool IsDirty() const { return dirty_; }

	// Reset for next frame
	void Clear() {
		renderPasses_.clear();
		computePasses_.clear();
		transferPasses_.clear();
		edges_.clear();
		resources_.Clear();
		compiled_ = CompiledRenderGraph{};
		dirty_ = true;
	}

private:
	void BuildDependencies() {
		edges_.clear();

		// Track which pass last wrote to each resource
		std::unordered_map<ResourceHandle, std::pair<std::uint32_t, ResourceUsage>> lastWriter;

		// Build unified pass list for dependency tracking
		struct UnifiedPass {
			PassHandle handle;
			std::vector<ResourceRef>* inputs;
			std::vector<ResourceRef>* outputs;
		};
		std::vector<UnifiedPass> allPasses;

		for (std::uint32_t i = 0; i < renderPasses_.size(); ++i) {
			allPasses.push_back({
				{PassType::Raster, i},
				&renderPasses_[i].inputs,
				&renderPasses_[i].colorOutputs
			});
		}

		for (std::uint32_t i = 0; i < computePasses_.size(); ++i) {
			allPasses.push_back({
				{PassType::Compute, i},
				&computePasses_[i].inputs,
				&computePasses_[i].outputs
			});
		}

		// For each pass, check if it reads from a resource that was written by a previous pass
		for (std::size_t passIdx = 0; passIdx < allPasses.size(); ++passIdx) {
			auto& pass = allPasses[passIdx];

			// Check inputs - create edges from writers
			if (pass.inputs) {
				for (const auto& input : *pass.inputs) {
					if (!input.IsValid()) continue;

					auto writerIt = lastWriter.find(input.handle);
					if (writerIt != lastWriter.end()) {
						RenderGraphEdge edge;
						edge.sourcePass = writerIt->second.first;
						edge.destPass = static_cast<std::uint32_t>(passIdx);
						edge.resource = input.handle;
						edge.sourceUsage = writerIt->second.second;
						edge.destUsage = input.usage;
						edges_.push_back(edge);
					}
				}
			}

			// Record outputs as writers
			if (pass.outputs) {
				for (const auto& output : *pass.outputs) {
					if (!output.IsValid()) continue;
					lastWriter[output.handle] = {static_cast<std::uint32_t>(passIdx), output.usage};
				}
			}
		}
	}

	bool TopologicalSort() {
		const std::size_t totalPasses = renderPasses_.size() + computePasses_.size() + transferPasses_.size();

		// Build adjacency list
		std::vector<std::vector<std::uint32_t>> adj(totalPasses);
		std::vector<std::uint32_t> inDegree(totalPasses, 0);

		for (const auto& edge : edges_) {
			if (edge.sourcePass < totalPasses && edge.destPass < totalPasses) {
				adj[edge.sourcePass].push_back(edge.destPass);
				inDegree[edge.destPass]++;
			}
		}

		// Kahn's algorithm
		std::queue<std::uint32_t> queue;
		for (std::uint32_t i = 0; i < totalPasses; ++i) {
			if (inDegree[i] == 0) {
				queue.push(i);
			}
		}

		compiled_.executionOrder.clear();
		compiled_.executionOrder.reserve(totalPasses);

		while (!queue.empty()) {
			const std::uint32_t current = queue.front();
			queue.pop();
			compiled_.executionOrder.push_back(current);

			for (const std::uint32_t neighbor : adj[current]) {
				if (--inDegree[neighbor] == 0) {
					queue.push(neighbor);
				}
			}
		}

		// Check for cycle
		return compiled_.executionOrder.size() == totalPasses;
	}

	void MergePasses() {
		compiled_.mergedGroups.clear();

		if (compiled_.executionOrder.empty()) {
			return;
		}

		// Only raster passes can be merged into subpasses
		// Passes can merge if:
		// 1. Both are raster passes
		// 2. Second pass reads from first pass's color/depth output
		// 3. Both have compatible render area (same resolution)
		// 4. No external dependencies break the merge

		std::vector<bool> merged(renderPasses_.size(), false);

		for (std::size_t i = 0; i < compiled_.executionOrder.size(); ++i) {
			const std::uint32_t passIdx = compiled_.executionOrder[i];

			// Only consider unmerged raster passes
			if (passIdx >= renderPasses_.size() || merged[passIdx]) {
				continue;
			}

			MergedPassGroup group;
			group.passIndices.push_back(passIdx);
			merged[passIdx] = true;

			// Collect outputs from first pass
			for (const auto& output : renderPasses_[passIdx].colorOutputs) {
				if (output.IsValid()) {
					group.colorAttachments.push_back(output.handle);
					group.allOutputs.push_back(output);
				}
			}

			// Try to merge subsequent passes
			for (std::size_t j = i + 1; j < compiled_.executionOrder.size(); ++j) {
				const std::uint32_t candidateIdx = compiled_.executionOrder[j];

				if (candidateIdx >= renderPasses_.size() || merged[candidateIdx]) {
					continue;
				}

				const auto& candidate = renderPasses_[candidateIdx];

				// Check if candidate reads from any of group's outputs
				bool canMerge = false;
				for (const auto& input : candidate.inputs) {
					for (const auto& attachment : group.colorAttachments) {
						if (input.handle == attachment) {
							canMerge = true;
							break;
						}
					}
					if (canMerge) break;
				}

				if (canMerge) {
					group.passIndices.push_back(candidateIdx);
					merged[candidateIdx] = true;

					// Add candidate's outputs to group
					for (const auto& output : candidate.colorOutputs) {
						if (output.IsValid()) {
							group.colorAttachments.push_back(output.handle);
							group.allOutputs.push_back(output);
						}
					}

					for (const auto& input : candidate.inputs) {
						if (input.IsValid()) {
							group.allInputs.push_back(input);
						}
					}
				}
			}

			compiled_.mergedGroups.push_back(std::move(group));
		}

		// Handle remaining unmerged raster passes
		for (std::uint32_t i = 0; i < renderPasses_.size(); ++i) {
			if (!merged[i]) {
				MergedPassGroup group;
				group.passIndices.push_back(i);
				for (const auto& output : renderPasses_[i].colorOutputs) {
					if (output.IsValid()) {
						group.colorAttachments.push_back(output.handle);
						group.allOutputs.push_back(output);
					}
				}
				compiled_.mergedGroups.push_back(std::move(group));
			}
		}
	}

	void ComputeAliasing() {
		// Build resource lifetime spans
		std::unordered_map<ResourceHandle, ResourceLifetime> lifetimes;

		for (std::size_t orderIdx = 0; orderIdx < compiled_.executionOrder.size(); ++orderIdx) {
			const std::uint32_t passIdx = compiled_.executionOrder[orderIdx];

			auto updateLifetime = [&](ResourceHandle handle, std::uint32_t executionIndex) {
				if (!resources_.IsTransient(handle)) return;

				auto& lifetime = lifetimes[handle];
				lifetime.handle = handle;
				lifetime.firstUse = std::min(lifetime.firstUse, executionIndex);
				lifetime.lastUse = std::max(lifetime.lastUse, executionIndex);
				lifetime.memorySize = resources_.GetResourceSize(handle);
			};

			// Process pass inputs/outputs
			if (passIdx < renderPasses_.size()) {
				for (const auto& input : renderPasses_[passIdx].inputs) {
					if (input.IsValid()) {
						updateLifetime(input.handle, static_cast<std::uint32_t>(orderIdx));
					}
				}
				for (const auto& output : renderPasses_[passIdx].colorOutputs) {
					if (output.IsValid()) {
						updateLifetime(output.handle, static_cast<std::uint32_t>(orderIdx));
					}
				}
			}
		}

		// Greedy aliasing - sort by size descending, then assign to groups
		std::vector<ResourceLifetime> sortedLifetimes;
		sortedLifetimes.reserve(lifetimes.size());
		for (auto& [handle, lifetime] : lifetimes) {
			sortedLifetimes.push_back(std::move(lifetime));
		}
		std::ranges::sort(sortedLifetimes, [](const auto& a, const auto& b) {
			return a.memorySize > b.memorySize;
		});

		std::uint32_t nextGroupId = 0;
		for (auto& lifetime : sortedLifetimes) {
			// Try to find existing group that this can alias with
			bool foundGroup = false;
			for (auto& group : compiled_.aliasingGroups) {
				// Check if this resource can alias with all members
				bool canAlias = true;
				for (const auto& memberHandle : group.members) {
					auto it = lifetimes.find(memberHandle);
					if (it != lifetimes.end() && !lifetime.CanAliasWith(it->second)) {
						canAlias = false;
						break;
					}
				}

				if (canAlias) {
					group.members.push_back(lifetime.handle);
					group.totalSize = std::max(group.totalSize, lifetime.memorySize);

					// Update transient resource aliasing info
					if (auto* transient = resources_.GetTransient(lifetime.handle)) {
						transient->aliasGroup = group.groupId;
						transient->aliasOffset = 0;  // Simplified: all start at 0 within group
					}

					foundGroup = true;
					break;
				}
			}

			if (!foundGroup) {
				AliasingGroup newGroup;
				newGroup.groupId = nextGroupId++;
				newGroup.totalSize = lifetime.memorySize;
				newGroup.members.push_back(lifetime.handle);

				if (auto* transient = resources_.GetTransient(lifetime.handle)) {
					transient->aliasGroup = newGroup.groupId;
					transient->aliasOffset = 0;
				}

				compiled_.aliasingGroups.push_back(std::move(newGroup));
			}
		}
	}

	void ComputeBarriers() {
		compiled_.barriers.clear();

		// For each edge, determine if a barrier is needed
		for (const auto& edge : edges_) {
			// Skip if same pass (shouldn't happen but safety check)
			if (edge.sourcePass == edge.destPass) continue;

			// Barrier needed if:
			// 1. Source wrote and dest reads (RAW hazard)
			// 2. Source read and dest writes (WAR hazard) - less common
			// 3. Both write (WAW hazard)

			const bool sourceWrites =
				HasFlag(edge.sourceUsage, ResourceUsage::ColorAttachmentWrite) ||
				HasFlag(edge.sourceUsage, ResourceUsage::DepthStencilWrite) ||
				HasFlag(edge.sourceUsage, ResourceUsage::ComputeShaderWrite) ||
				HasFlag(edge.sourceUsage, ResourceUsage::TransferDest);

			const bool destReads =
				HasFlag(edge.destUsage, ResourceUsage::FragmentShaderRead) ||
				HasFlag(edge.destUsage, ResourceUsage::VertexShaderRead) ||
				HasFlag(edge.destUsage, ResourceUsage::ComputeShaderRead) ||
				HasFlag(edge.destUsage, ResourceUsage::DepthStencilRead) ||
				HasFlag(edge.destUsage, ResourceUsage::TransferSource);

			if (sourceWrites || destReads) {
				CompiledRenderGraph::BarrierInfo barrier;
				barrier.afterPass = edge.sourcePass;
				barrier.beforePass = edge.destPass;
				barrier.resource = edge.resource;
				barrier.fromUsage = edge.sourceUsage;
				barrier.toUsage = edge.destUsage;
				compiled_.barriers.push_back(barrier);
			}
		}
	}

	// Pass storage
	std::vector<RenderPassDesc> renderPasses_;
	std::vector<ComputePassDesc> computePasses_;
	std::vector<TransferPassDesc> transferPasses_;

	// Dependency edges
	std::vector<RenderGraphEdge> edges_;

	// Resources
	ResourceRegistry resources_;

	// Compiled output
	CompiledRenderGraph compiled_;
	bool dirty_ = true;
};

// Render graph module base class (templated on scheduler for parallelism)
export template<SchedulerBackend SchedulerType>
class RenderGraphModule {
public:
	RenderGraphModule(std::shared_ptr<RasterModule>&, SchedulerType& scheduler) :
		taskGraph_(scheduler) {
	}

	virtual ~RenderGraphModule() = default;

	RenderGraphModule(const RenderGraphModule&) = delete;
	RenderGraphModule(RenderGraphModule&&) noexcept = default;

	RenderGraphModule& operator=(const RenderGraphModule&) = delete;
	RenderGraphModule& operator=(RenderGraphModule&&) noexcept = default;

	virtual void Execute() = 0;

	virtual void CreateRenderPass(RenderTaskParameters&,
		std::function<std::function<void(const EntityRegistry&, CommandList&)>(RenderGraphBuilder&)>) = 0;

	// Access the render graph for building
	[[nodiscard]] RenderGraph& GetRenderGraph() { return renderGraph_; }
	[[nodiscard]] const RenderGraph& GetRenderGraph() const { return renderGraph_; }

	// Factory
	static std::shared_ptr<RenderGraphModule<SchedulerType>> CreateModule(
		std::shared_ptr<RasterModule>& rasterModule,
		SchedulerType& scheduler,
		std::shared_ptr<EntityRegistry>& entityRegistry) {

		// TODO: Return concrete backend implementation
		return nullptr;
	}

protected:
	Graph<SchedulerType> taskGraph_;  // Parallelism task graph for async work
	RenderGraph renderGraph_;          // Render pass DAG
};
