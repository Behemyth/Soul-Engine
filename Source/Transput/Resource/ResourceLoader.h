#pragma once

#include <filesystem>

class ResourceLoader {

public:

	ResourceLoader() = default;
	virtual ~ResourceLoader() = default;

    ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader(ResourceLoader&&) noexcept = default;

	ResourceLoader& operator=(const ResourceLoader&) = delete;
	ResourceLoader& operator=(ResourceLoader&&) noexcept = default;

	virtual void Load(const std::string_view&) = 0;


};