export module synodic.soul.engine:core.interface.project;

import std;

export class Project {
public:
	Project();

	const std::filesystem::path& GetDirectory() const;

private:
	std::filesystem::path projectPath_;
};

// Implementation
inline Project::Project() {
}

inline const std::filesystem::path& Project::GetDirectory() const {
	return projectPath_;
}
