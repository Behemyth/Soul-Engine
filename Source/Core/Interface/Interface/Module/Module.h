#pragma once

#include "Core/Utility/Template/CRTP.h"

#include <unordered_map>
#include <memory>

//Common interface for modules.
template<typename T>
class Module : public CRTP<T, Module> {

public:
	
	virtual ~Module() = default;

	Module(const Module&) = delete;
	Module(Module&&) noexcept = default;

	Module& operator=(const Module&) = delete;
	Module& operator=(Module&&) noexcept = default;

	std::unique_ptr<T> Create(std::string hash);
	
protected:

	Module() = default;

private:

	static std::unordered_map<std::string, std::unique_ptr<T>> modules_;

};

template<typename T>
std::unique_ptr<T> Module<T>::Create(std::string hash)
{

	return modules_.at(hash);
	
}
