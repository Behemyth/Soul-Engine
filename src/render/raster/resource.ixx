export module synodic.soul.raster:resource;

import std;
import synodic.soul.core;

export enum class ResourceType {
	Buffer,
	Image
};

export class RenderResource : public Component {

public:

	RenderResource(ResourceType typeIn): type(typeIn)
	{
	}
	~RenderResource() = default;

	RenderResource(const RenderResource&) = delete;
	RenderResource(RenderResource&&) noexcept = default;

	RenderResource& operator=(const RenderResource&) = delete;
	RenderResource& operator=(RenderResource&&) noexcept = default;

	ResourceType type;

};

//TODO: Place somewhere else
export class RenderView : public Component {

public:

	float width;
	float height;
	float minDepth;
	float maxDepth;

};



export enum class BufferType {
	Vertex,
	Index,
	Uniform,
	Push,
	Storage
};

export class Buffer : public RenderResource {

public:

	Buffer(BufferType typeIn):
		RenderResource(ResourceType::Buffer),
		type(typeIn)
	{
	}
	~Buffer() = default;

	Buffer(const Buffer&) = delete;
	Buffer(Buffer&&) noexcept = default;

	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&&) noexcept = default;

private:

	BufferType type;


};

export class VertexBuffer : public Buffer{

public:

	VertexBuffer(): Buffer(BufferType::Vertex)
	{
	}
	~VertexBuffer() = default;

	VertexBuffer(const VertexBuffer&) = delete;
	VertexBuffer(VertexBuffer&&) noexcept = default;

	VertexBuffer& operator=(const VertexBuffer&) = delete;
	VertexBuffer& operator=(VertexBuffer&&) noexcept = default;

};

export class IndexBuffer : public Buffer {

public:

	IndexBuffer(): Buffer(BufferType::Index)
	{
	}
	~IndexBuffer() = default;

	IndexBuffer(const IndexBuffer&) = delete;
	IndexBuffer(IndexBuffer&&) noexcept = default;

	IndexBuffer& operator=(const IndexBuffer&) = delete;
	IndexBuffer& operator=(IndexBuffer&&) noexcept = default;

};

export class UniformBuffer : public Buffer {

public:

	UniformBuffer(): Buffer(BufferType::Uniform)
	{
	}
	~UniformBuffer() = default;

	UniformBuffer(const UniformBuffer&) = delete;
	UniformBuffer(UniformBuffer&&) noexcept = default;

	UniformBuffer& operator=(const UniformBuffer&) = delete;
	UniformBuffer& operator=(UniformBuffer&&) noexcept = default;
};

export class PushBuffer : public Buffer {

public:

	PushBuffer() : Buffer(BufferType::Push)
	{
	}
	~PushBuffer() = default;
	PushBuffer(const PushBuffer&) = delete;
	PushBuffer(PushBuffer&&) noexcept = default;

	PushBuffer& operator=(const PushBuffer&) = delete;
	PushBuffer& operator=(PushBuffer&&) noexcept = default;
};

export class StorageBuffer : public Buffer {


public:

	StorageBuffer(): Buffer(BufferType::Storage)
	{
	}
	~StorageBuffer() = default;

	StorageBuffer(const StorageBuffer&) = delete;
	StorageBuffer(StorageBuffer&&) noexcept = default;

	StorageBuffer& operator=(const StorageBuffer&) = delete;
	StorageBuffer& operator=(StorageBuffer&&) noexcept = default;
};
