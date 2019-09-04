#include "Entity.h"

Entity::Entity() :
	entity_(nullState)
{
}

Entity::Entity(value_type entity) :
	entity_(entity)
{
}

Entity::Entity(id_type id, version_type version) :
	entity_(version)
{

	entity_ <<= entityBitCount;
	entity_ |= id;

}

bool Entity::IsNull() const{
	return entity_ == nullState;
}

Entity::operator value_type() const {
	return entity_;
}

Entity::version_type Entity::GetVersion() const{
	return entity_ >> entityBitCount & versionMask;
}

Entity::id_type Entity::GetId() const {
	return entity_ & entityMask;
}

bool Entity::operator==(const Entity& other) const
{
	return entity_ == other.entity_;
}