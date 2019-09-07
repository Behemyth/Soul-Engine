#pragma once

#include "Event.h"
#include "Core/Utility/HashString/HashString.h"

#include <unordered_map>
#include <memory>

//TODO replace with Callable Concept c++20
#include <boost/callable_traits.hpp>

//signal and slots system
class EventRegistry final {

public:

	EventRegistry();
	~EventRegistry() = default;

	EventRegistry(const EventRegistry &) = delete;
	EventRegistry(EventRegistry &&) = default;

	EventRegistry & operator=(const EventRegistry &) = delete;
	EventRegistry & operator=(EventRegistry &&) = default;


	template<typename Fn>
	uint64 Listen(HashString::HashType, HashString::HashType, Fn&&);

	template <typename... Args>
	void Emit(HashString::HashType, HashString::HashType, Args&&...);

	void Remove(HashString::HashType, HashString::HashType, uint64);
	void Remove(HashString::HashType, HashString::HashType);
	void Remove(HashString::HashType);


private:

	using EventPtr = std::unique_ptr<BaseEvent>;
	using EventHashMap = std::unordered_map<
		HashString::HashType,
		std::unordered_map<HashString::HashType, EventPtr>
	>;

	//maps a hashedstring (as an int type) to an event
	EventHashMap eventMap_;
	uint64 idCounter_;

};

template<typename Fn>
uint64 EventRegistry::Listen(HashString::HashType channel,
	HashString::HashType name,
	Fn&& func)
{
	using type = boost::callable_traits::function_type_t<Fn>;

	auto& baseEventPtr = eventMap_[channel][name];
	if (!baseEventPtr.get()) {
		baseEventPtr = std::make_unique<Event<type>>();
	}

	auto event = static_cast<Event<type>*>(baseEventPtr.get());

	event->Listen(idCounter_, std::forward<Fn>(func));

	return idCounter_++;
}

template <typename... Args>
void EventRegistry::Emit(HashString::HashType channel, HashString::HashType name, Args&&... args)
{

	//TODO cast return type
	if (auto event = static_cast<Event<void(decltype(args)...)>*>(eventMap_[channel][name].get()); event)
	{
		event->Emit(std::forward<Args>(args)...);
	}

}