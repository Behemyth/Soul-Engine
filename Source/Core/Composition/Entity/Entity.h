#pragma once

#include "Types.h"

class EntityRegistry;

class Entity 
{

	//TODO: Replace with badge/attorney -client
	friend class EntityRegistry;
	friend struct std::hash<Entity>; 

	using value_type = uint64;
	using id_type = uint32;
	using version_type = uint32;

	static constexpr auto entityMask = 0xFFFFFFFF;
	static constexpr auto versionMask = 0xFFFFFFFF;
	static constexpr auto entityBitCount = 32;
	static constexpr auto nullState = entityMask;


public:

	//construction and assignment
	Entity();
	~Entity() = default;

	Entity(const Entity &) = default;
	Entity(Entity &&) noexcept = default;

	Entity& operator=(const Entity &) = default;
	Entity& operator=(Entity &&) noexcept = default;

	bool operator==(const Entity&) const;

	//public funcs
	bool IsNull() const;


private:

	//Valid Entities can only be created by the entity registry
	explicit Entity(value_type);
	explicit Entity(id_type, version_type);

	explicit operator value_type() const;

	version_type GetVersion() const;
	id_type GetId() const;

	value_type entity_;

};

namespace std {

	template<>
	struct hash<Entity> {
		std::size_t operator()(const Entity& entity) const
		{
			return static_cast<std::size_t>(entity.GetId());
		}
	};

}