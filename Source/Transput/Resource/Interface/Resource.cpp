#include "Resource.h"

Resource::Resource(const std::string_view resourcePath) : 
    path_(resourcePath) 
{

}

const std::filesystem::path& Resource::Path() const {

	return path_;

}
