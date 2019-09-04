#pragma once

#include "RenderGraphParameters.h"
#include "Render/Raster/RenderResource.h"
#include "Core/Composition/Entity/EntityRegistry.h"
#include "Render/Raster/RasterModule.h"

#include <vector>

class RenderGraphBuilder final{

public:

	RenderGraphBuilder(std::shared_ptr<RasterModule>&, std::shared_ptr<EntityRegistry>&, Entity, bool);
	~RenderGraphBuilder();

	RenderGraphBuilder(const RenderGraphBuilder &) = delete;
	RenderGraphBuilder(RenderGraphBuilder &&) noexcept = default;

	RenderGraphBuilder& operator=(const RenderGraphBuilder &) = delete;
	RenderGraphBuilder& operator=(RenderGraphBuilder &&) noexcept = default;

	void CreateOutput(RenderGraphOutputParameters&);
	void CreateInput(RenderGraphInputParameters&);

	void CreateSubpass();

	template<class T>
	Entity Request();

	Entity View();


private:

	std::shared_ptr<EntityRegistry> entityRegistry_;
	std::shared_ptr<RasterModule> rasterModule_;

	Entity renderPass_;
	bool subPass_;


};

template<class T>
Entity RenderGraphBuilder::Request()
{
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<RenderResource, T>::value,
		"The type parameter must be a subclass of RenderResource"); 


	Entity returnedEntity = entityRegistry_->CreateEntity();
	entityRegistry_->AttachComponent<T>(returnedEntity);

	return returnedEntity;

}
