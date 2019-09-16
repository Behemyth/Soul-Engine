#include "Project.h"

Project::Project() {
	
}


const std::filesystem::path& Project::GetDirectory() const {

	return projectPath_;

}