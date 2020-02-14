#pragma once

#include <BaseLibrary/Exception/Exception.hpp>

#include <cassert>
#include <filesystem>
#include <map>
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
	struct AssetParams {
		std::shared_ptr<AssetT> strongRef;
		std::filesystem::path filePath;
		std::filesystem::file_time_type lastWriteTime;
	};

public:
	virtual ~AssetCache() = default;

	/// <summary> Loads an asset from the file specified or from cache if already loaded. </summary>
	/// <param name="path"> Relative path to one of the search directories. </param>
	/// <exception cref="FileNotFoundException"> In case none of the search directories contain the file. </exception>
	std::shared_ptr<AssetT> Load(std::filesystem::path path);

	/// <summary> Reloads the asset from disk if the underlying file has changed. </summary>
	/// <exception cref="KeyNotFoundException"> If the file has not been loaded at all. Call Load first. </exception>
	std::shared_ptr<AssetT> Reload(std::filesystem::path path);

	/// <summary> Reloads the asset from disk if the underlying file has changed. </summary>
	/// <exception cref="KeyNotFoundException"> If the asset has not been found in the cache. </exception>
	void Reload(std::shared_ptr<AssetT> asset);

	/// <summary> Reloads every asset for which the source file has changed since it's been loaded. </summary>
	void ReloadAll();

	/// <summary> Releases the asset from the cache. </summary>
	void Release(std::filesystem::path path);

	/// <summary> Releases the asset from the cache. </summary>
	void Release(std::shared_ptr<AssetT> asset);

	/// <summary> Releases all resources from the cache. </summary>
	/// <remarks> The asset cache no longer keeps the assets around by strong references.
	///		If users of the resource keep the resource alive, it will still be loaded from the cache.
	///		If users have released all their references, it will be loaded from disk. </remarks>
	void ReleaseAll();

	/// <summary> Searches for assets within these directories. </summary>
	void SetSearchDirectories(std::vector<std::filesystem::path> directories);

protected:
	/// <summary> Loads the asset specified by the absolute path. </summary>
	virtual std::shared_ptr<AssetT> Create(const std::filesystem::path& path) = 0;
	virtual void Reload(AssetT& asset, const std::filesystem::path& path) = 0;

private:
	std::filesystem::path FindFullPath(std::filesystem::path path);
	std::vector<std::filesystem::path> m_directories;
	std::unordered_map<std::filesystem::path, std::weak_ptr<AssetT>, PathHash> m_pathMap;
	std::map<std::weak_ptr<AssetT>, AssetParams, std::owner_less<std::weak_ptr<AssetT>>> m_cache;
};


template <class AssetT>
std::shared_ptr<AssetT> AssetCache<AssetT>::Load(std::filesystem::path path) {
	auto pathMapIt = m_pathMap.find(path);
	if (pathMapIt != m_pathMap.end()) {
		std::weak_ptr<AssetT> weakRef = pathMapIt->second;
		std::shared_ptr<AssetT> strongRef = weakRef.lock();
		if (strongRef) {
			// Refresh strong reference in case it's been cleared.
			auto cacheIt = m_cache.find(weakRef);
			assert(cacheIt != m_cache.end());
			cacheIt->second.strongRef = strongRef;

			// Return strong reference.
			return strongRef;
		}
	}

	std::filesystem::path fullPath = FindFullPath(path).make_preferred();
	std::shared_ptr<AssetT> strongRef = Create(fullPath);
	std::weak_ptr<AssetT> weakRef = strongRef;
	auto lastWriteTime = last_write_time(fullPath);
	m_pathMap.insert_or_assign(path, weakRef);
	m_cache.insert_or_assign(weakRef, AssetParams{ .strongRef = strongRef, .filePath = path, .lastWriteTime = lastWriteTime });
	return strongRef;
}


template <class AssetT>
std::shared_ptr<AssetT> AssetCache<AssetT>::Reload(std::filesystem::path path) {
	auto pathMapIt = m_pathMap.find(path);
	if (pathMapIt != m_pathMap) {
		std::weak_ptr<AssetT> weakRef = pathMapIt->second;
		std::shared_ptr<AssetT> strongRef = weakRef.lock();
		if (strongRef) {
			Reload(strongRef);
			return strongRef;
		}
		else {
			m_pathMap.erase(pathMapIt);
			m_cache.erase(weakRef);
		}
	}
	return Load(path);
}


template <class AssetT>
void AssetCache<AssetT>::Reload(std::shared_ptr<AssetT> asset) {
	auto cacheIt = m_cache.find(asset);
	if (cacheIt != m_cache.end()) {
		AssetParams& params = cacheIt->second;
		params.strongRef = asset;
		std::filesystem::path fullPath = FindFullPath(params.path);
		auto freshWriteTime = last_write_time(fullPath);
		if (params.lastWriteTime < freshWriteTime) {
			Reload(*asset, fullPath);
		}
	}
	else {
		throw KeyNotFoundException{ "You can reload only an asset that was loaded previously by this cache." };
	}
}


template <class AssetT>
void AssetCache<AssetT>::ReloadAll() {
	for (auto& [weakRef, params] : m_cache) {
		if (!params.strongRef) {
			params.strongRef = weakRef.lock();
		}
		std::filesystem::path fullPath = FindFullPath(params.path);
		auto freshWriteTime = last_write_time(fullPath);
		if (params.strongRef && params.lastWriteTime < freshWriteTime) {
			Reload(*params.strongRef, fullPath);
		}
	}
}


template <class AssetT>
void AssetCache<AssetT>::Release(std::filesystem::path path) {
	auto pathMapIt = m_pathMap.find(path);
	if (pathMapIt != m_pathMap.end()) {
		auto cacheIt = m_cache.find(pathMapIt->second);
		assert(cacheIt != m_cache.end());
		cacheIt->second.strongRef.reset();
	}
}


template <class AssetT>
void AssetCache<AssetT>::Release(std::shared_ptr<AssetT> asset) {
	auto cacheIt = m_cache.find(asset);
	if (cacheIt != m_cache.end()) {
		cacheIt->second.strongRef.reset();
	}
	else {
		throw KeyNotFoundException{ "You can reload only an asset that was loaded previously by this cache." };
	}
}


template <class AssetT>
void AssetCache<AssetT>::ReleaseAll() {
	for (auto& [weakRef, params] : m_cache) {
		params.strongRef.reset();
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