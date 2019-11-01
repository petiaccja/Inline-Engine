#pragma once

#include <BaseLibrary/Exception/Exception.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>


namespace inl::asset {



template <class AssetT>
class AssetCache {
	struct PathHash {
		size_t operator()(const std::filesystem::path& obj) const {
			return hash_value(obj);
		}
	};
	struct AssetReference {
		std::weak_ptr<AssetT> m_reference;
		std::shared_ptr<AssetT> m_forced;
	};

public:
	virtual ~AssetCache() = default;

	/// <summary> Loads an asset from the file specified or from cache if already loaded. </summary>
	/// <param name="path"> Relative path to one of the search directories. </param>
	/// <exception cref="FileNotFoundException"> In case none of the search directories contain the file. </exception>
	std::shared_ptr<AssetT> Load(std::filesystem::path path);

	/// <summary> Searches for assets whithin these directories. </summary>
	void SetSearchDirectories(std::vector<std::filesystem::path> directories);

protected:
	/// <summary> Loads the asset specified by the absolute path. </summary>
	virtual std::shared_ptr<AssetT> Create(const std::filesystem::path& path) = 0;

private:
	std::filesystem::path FindFullPath(std::filesystem::path path);
	std::vector<std::filesystem::path> m_directories;
	std::unordered_map<std::filesystem::path, AssetReference, PathHash> m_cachedGraphicsMeshes;
};


template <class AssetT>
std::shared_ptr<AssetT> AssetCache<AssetT>::Load(std::filesystem::path path) {
	auto& cache = m_cachedGraphicsMeshes[path];
	try {
		if (!cache.m_forced) {
			cache.m_forced = cache.m_reference.lock();
		}
		if (!cache.m_forced) {
			cache.m_forced = Create(FindFullPath(path));
			cache.m_reference = cache.m_forced;
		}
		return cache.m_forced;
	}
	catch (...) {
		if (!cache.m_forced) {
			m_cachedGraphicsMeshes.erase(path);
		}
		throw;
	}
}


template <class AssetT>
void AssetCache<AssetT>::SetSearchDirectories(std::vector<std::filesystem::path> directories) {
	m_directories = std::move(directories);
}


template <class AssetT>
std::filesystem::path AssetCache<AssetT>::FindFullPath(std::filesystem::path path) {
	for (auto& directory : m_directories) {
		std::filesystem::path fullPath = directory / path;
		if (exists(fullPath)) {
			return fullPath;
		}
	}
	throw FileNotFoundException{ "File not found in any of the directories specified for assets files.", path.generic_string() };
}


} // namespace inl::asset