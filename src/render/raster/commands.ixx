export module synodic.soul.raster:commands;

import std;
import synodic.soul.core;
import :types;
import :resource;

// Temporary type until proper buffer types are available
template<typename T> using ExternalBuffer = std::vector<T>;

export enum class CommandType {
	Draw,
	DrawIndirect,
	UpdateBuffer,
	UpdateTexture,
	CopyBuffer,
	CopyTexture
};

export class RenderCommand{

public:

	RenderCommand() = default;
	~RenderCommand() = default;

};


export struct DrawCommand : RenderCommand {

	//draw
	std::uint32_t elementSize;
	std::uint32_t indexOffset;
	std::uint32_t vertexOffset;

	//scissor
	uvec2 scissorOffset;
	uvec2 scissorExtent;

	//data
	Entity* vertexBuffer;
	Entity* indexBuffer;

};

export struct DrawIndirectCommand : RenderCommand {

};

export struct UpdateBufferCommand : RenderCommand {

	std::uint32_t offset;
	ExternalBuffer<std::byte> data;
	Entity* buffer;

};

export struct UpdateTextureCommand : RenderCommand {

};

export struct CopyBufferCommand : RenderCommand {

};

export struct CopyTextureCommand : RenderCommand {

};
