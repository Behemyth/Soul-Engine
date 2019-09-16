#pragma once

#include "Entity.h"
#include "Core/Composition/Component/Component.h"
#include "Core/Utility/ID/ClassID.h"
#include "Core/Structure/Sparse/SparseHashMap.h"
#include "Core/Structure/Span.h"

#include <vector>
#include <memory>
#include <cassert>
#include <unordered_map>

class EntityModule{

	template<typename Comp>
	using storage_type = SparseHashMap<Entity, std::decay_t<Comp>>;

public:

	EntityModule();
	~EntityModule() = default;

	EntityModule(const EntityModule&) = delete;
	EntityModule(EntityModule&& o) = delete;

	EntityModule& operator=(const EntityModule&) = delete;
	EntityModule& operator=(EntityModule&& other) = delete;


	bool IsValid(Entity) const noexcept;

	Entity CreateEntity();
	//void RemoveEntity(Entity);


	template<typename Comp, typename... Args>
	void AttachComponent(Entity,
		Args&&...);

	template<typename Comp>
	void RemoveComponent();

	template<typename Comp>
	void RemoveComponent(Entity);

	template<typename Comp>
	Comp& GetComponent(Entity) const noexcept;

	template<typename... Comp>
	std::enable_if_t<bool(sizeof...(Comp) > 1), std::tuple<Comp&...>> GetComponent(Entity) const
		noexcept;
	
	template<typename Comp>
	nonstd::span<Comp> View();
	

private:

	std::vector<std::unique_ptr<SparseStructure>> componentPools_;
	std::vector<Entity> entities_;

	Entity::id_type availableEntities_;
	Entity::id_type nextAvailable_;


};

template<typename Comp>
Comp& EntityModule::GetComponent(Entity entity) const noexcept
{

	// TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	assert(IsValid(entity));

	const auto componentId = ClassID::ID<Comp>();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_.at(componentId).get());

	return pool.At(entity);

}

template<typename... Comp>
std::enable_if_t<bool(sizeof...(Comp) > 1), std::tuple<Comp&...>> EntityModule::GetComponent(
	Entity entity) const noexcept
{

	return std::tuple<Comp&...> {GetComponent<Comp>(entity)...};

}

template<typename Comp, typename... Args>
void EntityModule::AttachComponent(
	Entity entity,
	Args&&... args)
{

	//TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	assert(IsValid(entity));

	const auto componentId = ClassID::ID<Comp>();

	// componentId is always incrementing.
	if (componentId >= componentPools_.size()) {
		componentPools_.push_back(std::make_unique<SparseHashMap<Entity, Comp>>());
	}

	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_[componentId].get());

	pool.Emplace(entity, std::forward<Args>(args)...);

}

template<typename Comp>
void EntityModule::RemoveComponent()
{
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	const auto componentId = ClassID::Id<Comp>();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_[componentId].get());

	pool.Clear();

}

template<typename Comp>
void EntityModule::RemoveComponent(Entity entity)
{

	throw NotImplemented();

}

template<typename Comp>
nonstd::span<Comp> EntityModule::View()
{
	
	const auto componentId = ClassID::ID<Comp>();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_.at(componentId).get());
	
	return {pool.Data(), pool.Size()};
	
}
