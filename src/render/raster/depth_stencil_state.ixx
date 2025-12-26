/**
 * @file depth_stencil_state.ixx
 * @brief Separated depth-stencil state from pipeline state object
 * 
 * Following "No Graphics API" patterns, depth-stencil state is separated
 * from the main pipeline state object. This reduces PSO permutation explosion
 * and allows dynamic depth-stencil configuration per draw call.
 * 
 * Modern GPUs handle this efficiently via command buffer state rather than
 * baked pipeline state. Vulkan supports this via VK_EXT_extended_dynamic_state.
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:depth_stencil_state;

import std;

/**
 * @brief Comparison operation for depth and stencil tests
 */
export enum class CompareOp : std::uint8_t {
	Never,          ///< Always fail
	Less,           ///< Pass if src < dst
	Equal,          ///< Pass if src == dst
	LessOrEqual,    ///< Pass if src <= dst
	Greater,        ///< Pass if src > dst
	NotEqual,       ///< Pass if src != dst
	GreaterOrEqual, ///< Pass if src >= dst
	Always,         ///< Always pass
};

/**
 * @brief Stencil operation
 */
export enum class StencilOp : std::uint8_t {
	Keep,           ///< Keep current value
	Zero,           ///< Set to zero
	Replace,        ///< Replace with reference value
	IncrementClamp, ///< Increment and clamp to max
	DecrementClamp, ///< Decrement and clamp to zero
	Invert,         ///< Bitwise invert
	IncrementWrap,  ///< Increment with wrap
	DecrementWrap,  ///< Decrement with wrap
};

/**
 * @brief Per-face stencil operation state
 */
export struct StencilOpState {
	StencilOp failOp      = StencilOp::Keep;  ///< Stencil test fail
	StencilOp depthFailOp = StencilOp::Keep;  ///< Stencil pass, depth fail
	StencilOp passOp      = StencilOp::Keep;  ///< Both tests pass
	CompareOp compareOp   = CompareOp::Always;
	std::uint8_t compareMask = 0xFF;  ///< Mask for compare operation
	std::uint8_t writeMask   = 0xFF;  ///< Mask for write operation
	std::uint8_t reference   = 0;     ///< Reference value for compare
	
	constexpr StencilOpState() noexcept = default;
	
	constexpr bool operator==(const StencilOpState&) const noexcept = default;
	
	/**
	 * @brief Disabled stencil (always pass, no writes)
	 */
	[[nodiscard]] static constexpr StencilOpState Disabled() noexcept {
		return {};
	}
	
	/**
	 * @brief Replace on pass (for stencil masking)
	 */
	[[nodiscard]] static constexpr StencilOpState ReplaceOnPass(std::uint8_t ref = 1) noexcept {
		StencilOpState state;
		state.passOp = StencilOp::Replace;
		state.reference = ref;
		return state;
	}
	
	/**
	 * @brief Increment on pass (for stencil counting)
	 */
	[[nodiscard]] static constexpr StencilOpState IncrementOnPass() noexcept {
		StencilOpState state;
		state.passOp = StencilOp::IncrementClamp;
		return state;
	}
};

/**
 * @brief Complete depth-stencil state configuration
 * 
 * This state is set dynamically per draw call rather than baked into PSO.
 * Common configurations are provided as static factory methods.
 * 
 * Usage:
 * @code
 * // Standard opaque rendering
 * commands.SetDepthStencilState(DepthStencilState::DepthReadWrite());
 * 
 * // Transparent rendering (depth read, no write)
 * commands.SetDepthStencilState(DepthStencilState::DepthReadOnly());
 * 
 * // Sky/background (depth test disabled)
 * commands.SetDepthStencilState(DepthStencilState::Disabled());
 * @endcode
 */
export struct DepthStencilState {
	// Depth state
	bool depthTestEnable  = false;       ///< Enable depth testing
	bool depthWriteEnable = false;       ///< Enable depth buffer writes
	CompareOp depthCompareOp = CompareOp::Less;  ///< Depth comparison operation
	
	// Depth bounds (requires depthBoundsTestEnable feature)
	bool depthBoundsTestEnable = false;
	float minDepthBounds = 0.0f;
	float maxDepthBounds = 1.0f;
	
	// Stencil state
	bool stencilTestEnable = false;      ///< Enable stencil testing
	StencilOpState front{};              ///< Front-face stencil ops
	StencilOpState back{};               ///< Back-face stencil ops
	
	constexpr DepthStencilState() noexcept = default;
	
	constexpr bool operator==(const DepthStencilState&) const noexcept = default;
	
	// === Common presets ===
	
	/**
	 * @brief Depth and stencil testing disabled
	 * 
	 * Use for: UI, full-screen effects, sky rendering (draw last)
	 */
	[[nodiscard]] static constexpr DepthStencilState Disabled() noexcept {
		return {};
	}
	
	/**
	 * @brief Standard depth test with writes
	 * 
	 * Use for: Opaque geometry, depth pre-pass
	 * Default uses Less comparison (standard forward rendering)
	 */
	[[nodiscard]] static constexpr DepthStencilState DepthReadWrite(
		CompareOp compareOp = CompareOp::Less) noexcept 
	{
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = true;
		state.depthCompareOp = compareOp;
		return state;
	}
	
	/**
	 * @brief Depth test only (no writes)
	 * 
	 * Use for: Transparent geometry, decals
	 */
	[[nodiscard]] static constexpr DepthStencilState DepthReadOnly(
		CompareOp compareOp = CompareOp::Less) noexcept 
	{
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = false;
		state.depthCompareOp = compareOp;
		return state;
	}
	
	/**
	 * @brief Reverse-Z depth testing
	 * 
	 * Use for: Better depth precision with floating-point depth buffers
	 * Requires clearing depth to 0.0 instead of 1.0
	 */
	[[nodiscard]] static constexpr DepthStencilState ReverseZ() noexcept {
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = true;
		state.depthCompareOp = CompareOp::Greater;
		return state;
	}
	
	/**
	 * @brief Reverse-Z read-only
	 */
	[[nodiscard]] static constexpr DepthStencilState ReverseZReadOnly() noexcept {
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = false;
		state.depthCompareOp = CompareOp::Greater;
		return state;
	}
	
	/**
	 * @brief Depth equal test (for second pass with same geometry)
	 * 
	 * Use for: Multi-pass rendering, deferred lighting
	 */
	[[nodiscard]] static constexpr DepthStencilState DepthEqual() noexcept {
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = false;
		state.depthCompareOp = CompareOp::Equal;
		return state;
	}
	
	/**
	 * @brief Depth write only (no test)
	 * 
	 * Use for: Shadow map rendering, depth-only pre-pass
	 */
	[[nodiscard]] static constexpr DepthStencilState DepthWriteOnly() noexcept {
		DepthStencilState state;
		state.depthTestEnable = false;
		state.depthWriteEnable = true;
		return state;
	}
	
	/**
	 * @brief Stencil write mask
	 * 
	 * Use for: Writing stencil values for later masking
	 */
	[[nodiscard]] static constexpr DepthStencilState StencilWrite(
		std::uint8_t reference = 1) noexcept 
	{
		DepthStencilState state;
		state.stencilTestEnable = true;
		state.front = StencilOpState::ReplaceOnPass(reference);
		state.back = state.front;
		return state;
	}
	
	/**
	 * @brief Stencil test (read stencil, compare to reference)
	 * 
	 * Use for: Masked rendering based on stencil buffer
	 */
	[[nodiscard]] static constexpr DepthStencilState StencilTest(
		std::uint8_t reference = 1,
		CompareOp compareOp = CompareOp::Equal) noexcept 
	{
		DepthStencilState state;
		state.stencilTestEnable = true;
		state.front.compareOp = compareOp;
		state.front.reference = reference;
		state.front.writeMask = 0;  // No writes during test
		state.back = state.front;
		return state;
	}
	
	/**
	 * @brief Combined depth read-write with stencil write
	 * 
	 * Use for: Opaque geometry that also writes stencil
	 */
	[[nodiscard]] static constexpr DepthStencilState DepthAndStencilWrite(
		std::uint8_t stencilRef = 1,
		CompareOp depthCompare = CompareOp::Less) noexcept 
	{
		DepthStencilState state;
		state.depthTestEnable = true;
		state.depthWriteEnable = true;
		state.depthCompareOp = depthCompare;
		state.stencilTestEnable = true;
		state.front = StencilOpState::ReplaceOnPass(stencilRef);
		state.back = state.front;
		return state;
	}
	
	// === Builder pattern for custom configurations ===
	
	/**
	 * @brief Enable depth testing with specified comparison
	 */
	[[nodiscard]] constexpr DepthStencilState& WithDepthTest(CompareOp op = CompareOp::Less) noexcept {
		depthTestEnable = true;
		depthCompareOp = op;
		return *this;
	}
	
	/**
	 * @brief Enable depth writing
	 */
	[[nodiscard]] constexpr DepthStencilState& WithDepthWrite() noexcept {
		depthWriteEnable = true;
		return *this;
	}
	
	/**
	 * @brief Enable stencil with specified front/back operations
	 */
	[[nodiscard]] constexpr DepthStencilState& WithStencil(
		StencilOpState frontOps, 
		StencilOpState backOps) noexcept 
	{
		stencilTestEnable = true;
		front = frontOps;
		back = backOps;
		return *this;
	}
	
	/**
	 * @brief Enable stencil with same ops for front and back
	 */
	[[nodiscard]] constexpr DepthStencilState& WithStencil(StencilOpState ops) noexcept {
		return WithStencil(ops, ops);
	}
	
	/**
	 * @brief Enable depth bounds testing
	 */
	[[nodiscard]] constexpr DepthStencilState& WithDepthBounds(float minBound, float maxBound) noexcept {
		depthBoundsTestEnable = true;
		minDepthBounds = minBound;
		maxDepthBounds = maxBound;
		return *this;
	}
};

/**
 * @brief Command to set depth-stencil state dynamically
 */
export struct SetDepthStencilStateCommand {
	DepthStencilState state{};
	
	constexpr SetDepthStencilStateCommand() noexcept = default;
	
	constexpr explicit SetDepthStencilStateCommand(const DepthStencilState& s) noexcept 
		: state(s) 
	{}
};

