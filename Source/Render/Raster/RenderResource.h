#pragma once

#include "Types.h"
#include "Core/Composition/Component/Component.h"
#include "Core/Geometry/Vertex.h"

class RenderResource : public Component {

public:

	RenderResource() = default;
	~RenderResource() = default;

};

class RenderView : public RenderResource {

public:

	float width;
	float height;
	float minDepth;
	float maxDepth;

};

class Buffer : public RenderResource {

public:

	Buffer() = default;
	~Buffer() = default;

	uint size;

};

class VertexBuffer : public Buffer{

public:

	VertexBuffer() = default;
	~VertexBuffer() = default;

};

class IndexBuffer : public Buffer {

public:

	IndexBuffer() = default;
	~IndexBuffer() = default;

};

class UniformBuffer : public Buffer {

public:

	UniformBuffer() = default;
	~UniformBuffer() = default;

};

class PushBuffer : public Buffer {

public:

	PushBuffer() = default;
	~PushBuffer() = default;

};

class StorageBuffer : public Buffer {


public:

	StorageBuffer() = default;
	~StorageBuffer() = default;

};