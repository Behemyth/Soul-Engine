/**
 * @file blend_state.ixx
 * @brief Separated blend state from pipeline state object
 * 
 * Following "No Graphics API" patterns, blend state is separated
 * from the main pipeline state object. This reduces PSO permutation explosion
 * and allows dynamic blend configuration per draw call.
 * 
 * Requires VK_EXT_extended_dynamic_state3 for dynamic blend state.
 * 
 * @see https://www.sebastianaaltonen.com/blog/no-graphics-api
 */
export module synodic.soul.raster:blend_state;

import std;

/**
 * @brief Blend factor for source and destination
 */
export enum class BlendFactor : std::uint8_t {
	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	Src1Color,
	OneMinusSrc1Color,
	Src1Alpha,
	OneMinusSrc1Alpha,
};

/**
 * @brief Blend operation
 */
export enum class BlendOp : std::uint8_t {
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max,
};

/**
 * @brief Color write mask flags
 */
export enum class ColorWriteMask : std::uint8_t {
	None  = 0,
	R     = 1 << 0,
	G     = 1 << 1,
	B     = 1 << 2,
	A     = 1 << 3,
	RGB   = R | G | B,
	RGBA  = R | G | B | A,
	All   = RGBA,
};

export constexpr ColorWriteMask operator|(ColorWriteMask a, ColorWriteMask b) noexcept {
	return static_cast<ColorWriteMask>(
		static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b));
}

export constexpr ColorWriteMask operator&(ColorWriteMask a, ColorWriteMask b) noexcept {
	return static_cast<ColorWriteMask>(
		static_cast<std::uint8_t>(a) & static_cast<std::uint8_t>(b));
}

export constexpr bool HasColorMask(ColorWriteMask flags, ColorWriteMask mask) noexcept {
	return (static_cast<std::uint8_t>(flags) & static_cast<std::uint8_t>(mask)) != 0;
}

/**
 * @brief Per-attachment blend state
 */
export struct BlendAttachmentState {
	bool blendEnable = false;
	BlendFactor srcColorBlendFactor = BlendFactor::One;
	BlendFactor dstColorBlendFactor = BlendFactor::Zero;
	BlendOp colorBlendOp = BlendOp::Add;
	BlendFactor srcAlphaBlendFactor = BlendFactor::One;
	BlendFactor dstAlphaBlendFactor = BlendFactor::Zero;
	BlendOp alphaBlendOp = BlendOp::Add;
	ColorWriteMask colorWriteMask = ColorWriteMask::All;
	
	constexpr BlendAttachmentState() noexcept = default;
	
	constexpr bool operator==(const BlendAttachmentState&) const noexcept = default;
	
	/**
	 * @brief Disabled blending (pass-through)
	 */
	[[nodiscard]] static constexpr BlendAttachmentState Disabled() noexcept {
		return {};
	}
	
	/**
	 * @brief Standard alpha blending: src.rgb * src.a + dst.rgb * (1 - src.a)
	 */
	[[nodiscard]] static constexpr BlendAttachmentState AlphaBlend() noexcept {
		BlendAttachmentState state;
		state.blendEnable = true;
		state.srcColorBlendFactor = BlendFactor::SrcAlpha;
		state.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
		state.colorBlendOp = BlendOp::Add;
		state.srcAlphaBlendFactor = BlendFactor::One;
		state.dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
		state.alphaBlendOp = BlendOp::Add;
		return state;
	}
	
	/**
	 * @brief Premultiplied alpha: src.rgb + dst.rgb * (1 - src.a)
	 */
	[[nodiscard]] static constexpr BlendAttachmentState Premultiplied() noexcept {
		BlendAttachmentState state;
		state.blendEnable = true;
		state.srcColorBlendFactor = BlendFactor::One;
		state.dstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
		state.colorBlendOp = BlendOp::Add;
		state.srcAlphaBlendFactor = BlendFactor::One;
		state.dstAlphaBlendFactor = BlendFactor::OneMinusSrcAlpha;
		state.alphaBlendOp = BlendOp::Add;
		return state;
	}
	
	/**
	 * @brief Additive blending: src.rgb + dst.rgb
	 */
	[[nodiscard]] static constexpr BlendAttachmentState Additive() noexcept {
		BlendAttachmentState state;
		state.blendEnable = true;
		state.srcColorBlendFactor = BlendFactor::One;
		state.dstColorBlendFactor = BlendFactor::One;
		state.colorBlendOp = BlendOp::Add;
		state.srcAlphaBlendFactor = BlendFactor::One;
		state.dstAlphaBlendFactor = BlendFactor::One;
		state.alphaBlendOp = BlendOp::Add;
		return state;
	}
	
	/**
	 * @brief Multiplicative blending: src.rgb * dst.rgb
	 */
	[[nodiscard]] static constexpr BlendAttachmentState Multiply() noexcept {
		BlendAttachmentState state;
		state.blendEnable = true;
		state.srcColorBlendFactor = BlendFactor::DstColor;
		state.dstColorBlendFactor = BlendFactor::Zero;
		state.colorBlendOp = BlendOp::Add;
		state.srcAlphaBlendFactor = BlendFactor::DstAlpha;
		state.dstAlphaBlendFactor = BlendFactor::Zero;
		state.alphaBlendOp = BlendOp::Add;
		return state;
	}
};

/**
 * @brief Complete blend state configuration
 * 
 * Supports multiple render targets with independent blend states.
 * 
 * Usage:
 * @code
 * // Standard alpha blending
 * commands.SetBlendState(BlendState::AlphaBlend());
 * 
 * // Opaque rendering (no blend)
 * commands.SetBlendState(BlendState::Opaque());
 * 
 * // Additive particles
 * commands.SetBlendState(BlendState::Additive());
 * @endcode
 */
export struct BlendState {
	static constexpr std::size_t MaxAttachments = 8;
	
	std::array<BlendAttachmentState, MaxAttachments> attachments{};
	std::uint32_t attachmentCount = 1;
	
	// Blend constants for BlendFactor::ConstantColor/Alpha
	float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	
	// Logic op (rarely used, disabled by default)
	bool logicOpEnable = false;
	
	constexpr BlendState() noexcept = default;
	
	constexpr bool operator==(const BlendState&) const noexcept = default;
	
	// === Common presets ===
	
	/**
	 * @brief Opaque rendering (no blending)
	 */
	[[nodiscard]] static constexpr BlendState Opaque() noexcept {
		BlendState state;
		state.attachments[0] = BlendAttachmentState::Disabled();
		return state;
	}
	
	/**
	 * @brief Standard alpha blending
	 */
	[[nodiscard]] static constexpr BlendState AlphaBlend() noexcept {
		BlendState state;
		state.attachments[0] = BlendAttachmentState::AlphaBlend();
		return state;
	}
	
	/**
	 * @brief Premultiplied alpha blending
	 */
	[[nodiscard]] static constexpr BlendState Premultiplied() noexcept {
		BlendState state;
		state.attachments[0] = BlendAttachmentState::Premultiplied();
		return state;
	}
	
	/**
	 * @brief Additive blending (particles, lights)
	 */
	[[nodiscard]] static constexpr BlendState Additive() noexcept {
		BlendState state;
		state.attachments[0] = BlendAttachmentState::Additive();
		return state;
	}
	
	/**
	 * @brief Multiplicative blending
	 */
	[[nodiscard]] static constexpr BlendState Multiply() noexcept {
		BlendState state;
		state.attachments[0] = BlendAttachmentState::Multiply();
		return state;
	}
	
	/**
	 * @brief No color writes (depth/stencil only)
	 */
	[[nodiscard]] static constexpr BlendState NoColorWrite() noexcept {
		BlendState state;
		state.attachments[0].colorWriteMask = ColorWriteMask::None;
		return state;
	}
	
	/**
	 * @brief MRT with same blend for all attachments
	 */
	[[nodiscard]] static constexpr BlendState MRT(
		BlendAttachmentState attachment, 
		std::uint32_t count) noexcept 
	{
		BlendState state;
		state.attachmentCount = count;
		for (std::uint32_t i = 0; i < count && i < MaxAttachments; ++i) {
			state.attachments[i] = attachment;
		}
		return state;
	}
	
	// === Builder pattern ===
	
	[[nodiscard]] constexpr BlendState& WithAttachment(
		std::uint32_t index, 
		BlendAttachmentState attachment) noexcept 
	{
		if (index < MaxAttachments) {
			attachments[index] = attachment;
			attachmentCount = std::max(attachmentCount, index + 1);
		}
		return *this;
	}
	
	[[nodiscard]] constexpr BlendState& WithBlendConstants(
		float r, float g, float b, float a) noexcept 
	{
		blendConstants[0] = r;
		blendConstants[1] = g;
		blendConstants[2] = b;
		blendConstants[3] = a;
		return *this;
	}
};

/**
 * @brief Command to set blend state dynamically
 */
export struct SetBlendStateCommand {
	BlendState state{};
	
	constexpr SetBlendStateCommand() noexcept = default;
	
	constexpr explicit SetBlendStateCommand(const BlendState& s) noexcept 
		: state(s) 
	{}
};

