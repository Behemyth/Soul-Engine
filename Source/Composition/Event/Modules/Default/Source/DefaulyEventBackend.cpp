#include "EventRegistry.h"

EventRegistry::EventRegistry() :
	idCounter_(0)
{
}

void EventRegistry::Remove(HashString::HashType channel, HashString::HashType name, uint64 id)
{
	eventMap_[channel][name]->Remove(id);
}

void EventRegistry::Remove(HashString::HashType channel, HashString::HashType name) {
	eventMap_[channel].erase(name);
}

void EventRegistry::Remove(HashString::HashType channel) {
	eventMap_.erase(channel);
}