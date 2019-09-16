#pragma once

#include <filesystem>


class Project {
	
public:

	Project();

    const std::filesystem::path& GetDirectory() const;


private:

	std::filesystem::path projectPath_;


};
