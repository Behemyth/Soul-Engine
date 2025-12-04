export module synodic.soul.core:entity;

import std;
import :types;
import :utility;
import :component;

// Entity class
export class Entity {
	friend class EntityRegistry;
	friend struct std::hash<Entity>;

	using value_type = std::int64_t;
	using id_type = std::uint32_t;
	using version_type = std::uint32_t;

	static constexpr auto entityMask = 0xFFFFFFFF;
	static constexpr auto versionMask = 0xFFFFFFFF;
	static constexpr auto entityBitCount = 32;
	static constexpr auto nullState = entityMask;

public:
	// Construction and assignment
	Entity();
	~Entity() = default;

	Entity(const Entity&) = default;
	Entity(Entity&&) noexcept = default;

	Entity& operator=(const Entity&) = default;
	Entity& operator=(Entity&&) noexcept = default;

	bool operator==(const Entity&) const;

	// Public functions
	bool IsNull() const;

private:
	// Valid Entities can only be created by the entity registry
	explicit Entity(value_type);
	explicit Entity(id_type, version_type);

	explicit operator value_type() const;

	version_type GetVersion() const;
	id_type GetId() const;

	value_type entity_;
};

// Entity implementation
Entity::Entity() : entity_(nullState) {
}

Entity::Entity(value_type entity) : entity_(entity) {
}

Entity::Entity(id_type id, version_type version) : entity_(version) {
	entity_ <<= entityBitCount;
	entity_ |= id;
}

bool Entity::IsNull() const {
	return entity_ == nullState;
}

Entity::operator value_type() const {
	return entity_;
}

Entity::version_type Entity::GetVersion() const {
	return entity_ >> entityBitCount & versionMask;
}

Entity::id_type Entity::GetId() const {
	return entity_ & entityMask;
}

bool Entity::operator==(const Entity& other) const {
	return entity_ == other.entity_;
}

// std::hash specialization for Entity
export namespace std {
	template<>
	struct hash<Entity> {
		std::size_t operator()(const Entity& entity) const {
			return static_cast<std::size_t>(entity.GetId());
		}
	};
}

// SparseStructure base class for type erasure
export class SparseStructure {
public:
	virtual ~SparseStructure() = default;
	virtual void Clear() = 0;
	virtual void Erase(Entity) = 0;
};

// SparseHashMap implementation
export template<typename Key, typename Value>
class SparseHashMap : public SparseStructure {
public:
	SparseHashMap() = default;
	~SparseHashMap() override = default;

	void Clear() override {
		data_.clear();
	}

	void Erase(Entity key) override {
		data_.erase(key);
	}

	template<typename... Args>
	void Emplace(const Key& key, Args&&... args) {
		data_.try_emplace(key, std::forward<Args>(args)...);
	}

	Value& At(const Key& key) {
		return data_.at(key);
	}

	const Value& At(const Key& key) const {
		return data_.at(key);
	}

	Value* Data() {
		if (data_.empty()) return nullptr;
		return &data_.begin()->second;
	}

	std::size_t Size() const {
		return data_.size();
	}

	bool Contains(const Key& key) const {
		return data_.contains(key);
	}

private:
	std::unordered_map<Key, Value> data_;
};

// EntityRegistry class
export class EntityRegistry {
	template<typename Comp>
	using storage_type = SparseHashMap<Entity, std::decay_t<Comp>>;

public:
	EntityRegistry();
	~EntityRegistry() = default;

	EntityRegistry(const EntityRegistry&) = delete;
	EntityRegistry(EntityRegistry&& o) = delete;

	EntityRegistry& operator=(const EntityRegistry&) = delete;
	EntityRegistry& operator=(EntityRegistry&& other) = delete;

	bool IsValid(Entity) const noexcept;

	Entity CreateEntity();
	// void RemoveEntity(Entity);

	template<typename Comp, typename... Args>
	void AttachComponent(Entity, Args&&...);

	template<typename Comp>
	void RemoveComponent();

	template<typename Comp>
	void RemoveComponent(Entity);

	template<typename Comp>
	Comp& GetComponent(Entity) const noexcept;

	template<typename... Comp>
	std::enable_if_t<bool(sizeof...(Comp) > 1), std::tuple<Comp&...>> GetComponent(Entity) const noexcept;

	template<typename Comp>
	std::span<Comp> View();

private:
	std::vector<std::unique_ptr<SparseStructure>> componentPools_;
	std::vector<Entity> entities_;

	Entity::id_type availableEntities_;
	Entity::id_type nextAvailable_;
};

// EntityRegistry constructor
EntityRegistry::EntityRegistry() : availableEntities_(0), nextAvailable_(0) {
}

// IsValid implementation
bool EntityRegistry::IsValid(Entity entity) const noexcept {
	const auto id = entity.GetId();
	return id < entities_.size() && entities_[id].entity_ == entity.entity_;
}

// CreateEntity implementation
Entity EntityRegistry::CreateEntity() {
	Entity entityID;

	// if no entities are available for reuse, create a new one
	if (availableEntities_) {
		const auto id = nextAvailable_;
		nextAvailable_ = entities_[id].GetId();
		const auto version = entities_[id].GetVersion();

		entityID = Entity(id, version);
		entities_[id] = entityID;
		--availableEntities_;
	}
	else {
		// simply add an entity. No max entity size check, who will ever go past std::uint32_t entities? ;P
		entityID = Entity(entities_.size());
		entities_.push_back(entityID);
	}

	return entityID;
}

// Template member function implementations

template<typename Comp>
Comp& EntityRegistry::GetComponent(Entity entity) const noexcept {
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	// assert(IsValid(entity)); // TODO: Replace with proper error handling

	const auto componentId = ClassID<Comp>::ID();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_.at(componentId).get());

	return pool.At(entity);
}

template<typename... Comp>
std::enable_if_t<bool(sizeof...(Comp) > 1), std::tuple<Comp&...>> EntityRegistry::GetComponent(
	Entity entity) const noexcept {
	return std::tuple<Comp&...>{GetComponent<Comp>(entity)...};
}

template<typename Comp, typename... Args>
void EntityRegistry::AttachComponent(Entity entity, Args&&... args) {
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	// assert(IsValid(entity)); // TODO: Replace with proper error handling

	const auto componentId = ClassID<Comp>::ID();

	// componentId is always incrementing.
	if (componentId >= componentPools_.size()) {
		componentPools_.push_back(std::make_unique<SparseHashMap<Entity, Comp>>());
	}

	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_[componentId].get());

	pool.Emplace(entity, std::forward<Args>(args)...);
}

template<typename Comp>
void EntityRegistry::RemoveComponent() {
	// TODO: C++20 Concepts
	static_assert(std::is_base_of<Component, Comp>::value,
		"The Comp parameter must be a subclass of Component");

	const auto componentId = ClassID<Comp>::ID();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_[componentId].get());

	pool.Clear();
}

template<typename Comp>
void EntityRegistry::RemoveComponent(Entity entity) {
	throw std::runtime_error("EntityRegistry::RemoveComponent not yet implemented");
}

template<typename Comp>
std::span<Comp> EntityRegistry::View() {
	const auto componentId = ClassID<Comp>::ID();
	auto& pool = *static_cast<storage_type<Comp>*>(componentPools_.at(componentId).get());

	return {pool.Data(), pool.Size()};
}

// TODO: Reimplement without type info?
// void EntityRegistry::RemoveEntity(Entity entity)
// {
// 	assert(IsValid(entity));
//
// 	for (auto& pool : componentPools_) {
// 		pool->Erase(entity);
// 	}
//
// 	// grab entity data
// 	const auto id = availableEntities_ ? nextAvailable_ : entity.GetId() + 1;
// 	const auto version = entity.GetVersion() + 1;
//
// 	// set the incremented version
// 	entities_[id] = Entity(id, version);
// 	nextAvailable_ = id;
// 	++availableEntities_;
// }
