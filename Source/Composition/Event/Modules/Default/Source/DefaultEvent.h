#pragma once

#include "Types.h"

#include <functional>
#include <unordered_map>


class BaseEvent
{

public:

	BaseEvent() = default;
	virtual ~BaseEvent() = default;

	virtual void Remove(uint64) {}

};

//force specialization
template<typename>
class Event;

template<typename R, typename... Types>
class Event<R(Types...)> : public BaseEvent
{

public:

	using signature = std::function<R(Types...)>;

	Event() = default;
	~Event() = default;

	Event(const Event &) = delete;
	Event(Event &&) = default;

	Event & operator=(const Event &) = delete;
	Event & operator=(Event &&) = default;

	//stores the callable with quick lookup enabled by the ID
	void Listen(uint64, signature&&);
	void Remove(uint64) override;
	void RemoveAll() const;

	//calls all stored callables
	template<typename... Args>
	void Emit(Args&&... args);

private:

	//the hashmap of 
	mutable std::unordered_map<uint64, signature> listeners;

};


template<typename R, typename ... Types>
void Event<R(Types...)>::Listen(uint64 id, signature&& fn) {
	listeners.insert(std::make_pair(id, fn));
}

template<typename R, typename ... Types>
void Event<R(Types...)>::Remove(uint64 id) {
	listeners.erase(id);
}

template<typename R, typename ... Types>
void Event<R(Types...)>::RemoveAll() const {
	listeners.clear();
}

template<typename R, typename ... Types>
template<typename... Args>
void Event<R(Types...)>::Emit(Args&&... args) {
	for (const auto&[key, value] : listeners) {
		std::invoke(value, std::forward<Args>(args)...);
	}
}