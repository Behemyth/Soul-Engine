export module synodic.soul.core:event;

import :types;
import :utility;
import std;

export class BaseEvent
{

public:

	BaseEvent() = default;
	virtual ~BaseEvent() = default;

	virtual void Remove(std::int64_t) {}

};

//force specialization
export template<typename>
class Event;

export template<typename R, typename... Types>
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
	void Listen(std::int64_t, signature&&);
	void Remove(std::int64_t) override;
	void RemoveAll() const;

	//calls all stored callables
	template<typename... Args>
	void Emit(Args&&... args);

private:

	//the hashmap of
	mutable std::unordered_map<std::int64_t, signature> listeners;

};


template<typename R, typename ... Types>
void Event<R(Types...)>::Listen(std::int64_t id, signature&& fn) {
	listeners.insert(std::make_pair(id, fn));
}

template<typename R, typename ... Types>
void Event<R(Types...)>::Remove(std::int64_t id) {
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

// Helper to extract function signature from callable types
namespace detail {
	// For function pointers
	template<typename T>
	struct function_signature;

	template<typename R, typename... Args>
	struct function_signature<R(*)(Args...)> {
		using type = R(Args...);
	};

	// For member function pointers
	template<typename R, typename C, typename... Args>
	struct function_signature<R(C::*)(Args...)> {
		using type = R(Args...);
	};

	template<typename R, typename C, typename... Args>
	struct function_signature<R(C::*)(Args...) const> {
		using type = R(Args...);
	};

	// For lambdas and functors with operator()
	template<typename T>
	struct function_signature : function_signature<decltype(&T::operator())> {};

	// For std::function
	template<typename R, typename... Args>
	struct function_signature<std::function<R(Args...)>> {
		using type = R(Args...);
	};

	template<typename T>
	using function_signature_t = typename function_signature<std::decay_t<T>>::type;
}

//signal and slots system
export class EventRegistry final {

public:

	EventRegistry();
	~EventRegistry() = default;

	EventRegistry(const EventRegistry &) = delete;
	EventRegistry(EventRegistry &&) = default;

	EventRegistry & operator=(const EventRegistry &) = delete;
	EventRegistry & operator=(EventRegistry &&) = default;


	template<typename Fn>
	std::int64_t Listen(HashString::HashType, HashString::HashType, Fn&&);

	template <typename... Args>
	void Emit(HashString::HashType, HashString::HashType, Args&&...);

	void Remove(HashString::HashType, HashString::HashType, std::int64_t);
	void Remove(HashString::HashType, HashString::HashType);
	void Remove(HashString::HashType);


private:

	using EventPtr = std::unique_ptr<BaseEvent>;
	using EventHashMap = std::unordered_map<
		HashString::HashType,
		std::unordered_map<HashString::HashType, EventPtr>
	>;

	//maps a hashedstring (as an std::int32_t type) to an event
	EventHashMap eventMap_;
	std::int64_t idCounter_;

};

template<typename Fn>
std::int64_t EventRegistry::Listen(HashString::HashType channel,
	HashString::HashType name,
	Fn&& func)
{
	using type = detail::function_signature_t<Fn>;

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

inline EventRegistry::EventRegistry() :
	idCounter_(0)
{
}

inline void EventRegistry::Remove(HashString::HashType channel, HashString::HashType name, std::int64_t listener)
{

	eventMap_[channel][name]->Remove(listener);

}

inline void EventRegistry::Remove(HashString::HashType channel, HashString::HashType name)
{

	eventMap_[channel].erase(name);

}

inline void EventRegistry::Remove(HashString::HashType channel)
{

	eventMap_.erase(channel);

}
