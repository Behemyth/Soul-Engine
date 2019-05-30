#pragma once

#include <filesystem>


enum class ResourceType {
    Image,
    Shader
};

class Resource {
	
public:

	Resource(std::string_view);
	~Resource() = default;

    const std::filesystem::path& Path() const;


private:

    std::filesystem::path path_;


};
