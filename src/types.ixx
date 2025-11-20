export module synodic.soul.core:types;

import std;

// TypeID - Simple type identification via CRTP
export template<typename T>
class TypeID {
public:
	static size_t GetTypeID() {
		static size_t id = reinterpret_cast<size_t>(&id);
		return id;
	}

protected:
	TypeID() = default;
	~TypeID() = default;
};



// Property template
export template<typename T>
class Property {
public:
	Property(const T&);
	~Property() = default;

	Property& operator=(const T&);
	operator T() const;

	void AddCallback(const std::function<void(T)>&);
	void Update();

private:
	T data;
	std::vector<std::function<void(T)>> callbacks;
};

export template<typename T>
Property<T>::Property(const T& value) {
	data = value;
}

export template<typename T>
Property<T>& Property<T>::operator=(const T& value) {
	data = value;
	Update();
	return *this;
}

export template<typename T>
void Property<T>::Update() {
	for (const auto& callback : callbacks) {
		callback(data);
	}
}

export template<typename T>
Property<T>::operator T() const {
	return data;
}

export template<typename T>
void Property<T>::AddCallback(const std::function<void(T)>& fn) {
	callbacks.push_back(fn);
}
