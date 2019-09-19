#pragma once

#include <functional>

template<typename T>
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

template<typename T>
Property<T>::Property(const T& value) {
	data = value;
}


template<typename T>
Property<T>& Property<T>::operator=(const T& value){

	data = value;
	Update();

	return *this;

}

template<typename T>
void Property<T>::Update() {

	for (const auto& callback : callbacks) {
		callback(data);
	}

}

template<typename T>
Property<T>::operator T() const {

	return data;

}

template<typename T>
void Property<T>::AddCallback(const std::function<void(T)>& fn) {

	callbacks.push_back(fn);

}