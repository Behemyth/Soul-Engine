export module synodic.soul.transput:resource;

import std;

export class Resource {
public:
	Resource(std::string_view);
	~Resource() = default;

	const std::filesystem::path& Path() const;

private:
	std::filesystem::path path_;
};

export class Asset {
public:
	Asset() = default;
	~Asset() = default;
};

export class ResourceLoader {
public:
	ResourceLoader() = default;
	virtual ~ResourceLoader() = default;

	ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader(ResourceLoader&&) noexcept = default;

	ResourceLoader& operator=(const ResourceLoader&) = delete;
	ResourceLoader& operator=(ResourceLoader&&) noexcept = default;

	virtual void Load(const std::string_view&) = 0;
};

inline Resource::Resource(const std::string_view resourcePath) :
	path_(resourcePath) {
}

inline const std::filesystem::path& Resource::Path() const {
	return path_;
}
