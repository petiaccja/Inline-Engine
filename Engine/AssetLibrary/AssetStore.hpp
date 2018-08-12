#pragma once

#include <unordered_map>
#include <memory>
#include <string_view>
#include <filesystem>

#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/MaterialShader.hpp>

#undef LoadImage



namespace inl::asset {



class AssetStore {
public:
	AssetStore(gxeng::GraphicsEngine* graphicsEngine);

	/// <summary> Loads a model file from a common format such as FBX. </summary>
	std::shared_ptr<gxeng::Mesh> LoadMesh(std::filesystem::path path);

	/// <summary> Loads an image file from a common format such as JPG or TIF. </summary>
	std::shared_ptr<gxeng::Image> LoadImage(std::filesystem::path path);

	/// <summary> Loads a material shader from a JSON graph description. </summary>
	std::shared_ptr<gxeng::MaterialShader> LoadMaterialShader(std::filesystem::path path);

	/// <summary> Loads a material from a JSON material description. </summary>
	std::shared_ptr<gxeng::Material> LoadMaterial(std::filesystem::path path);

	/// <summary> Adds a new source directory to look for assets. </summary>
	void AddSourceDirectory(std::filesystem::path directory);

	/// <summary> Removes a asset directory from the list. </summary>
	void RemoveSourceDirectory(std::filesystem::path directory);

	/// <summary> Removes all added asset directories. </summary>
	void ClearSourceDirectories();

private:
	std::shared_ptr<gxeng::Mesh> ForceLoadMesh(std::filesystem::path path);
	std::shared_ptr<gxeng::Image> ForceLoadImage(std::filesystem::path path);
	std::shared_ptr<gxeng::MaterialShader> ForceLoadMaterialShader(std::filesystem::path path);
	std::shared_ptr<gxeng::Material> ForceLoadMaterial(std::filesystem::path path);

	void SetMaterialParameter(gxeng::Material::Parameter& param, std::string value);
	void SetMaterialParameter(gxeng::Material::Parameter& param, float value);

	std::filesystem::path GetFullPath(std::filesystem::path localPath) const;
private:
	struct PathHash {
		size_t operator()(const std::filesystem::path& obj) const {
			return std::filesystem::hash_value(obj);
		}
	};
	template <class T>
	struct CachedAsset {
		std::weak_ptr<T> m_reference;
		std::shared_ptr<T> m_forced;
	};
private:
	std::unordered_set<std::filesystem::path, PathHash> m_directories;
	std::unordered_map<std::filesystem::path, CachedAsset<gxeng::Mesh>, PathHash> m_cachedMeshes;
	std::unordered_map<std::filesystem::path, CachedAsset<gxeng::Image>, PathHash> m_cachedImages;
	std::unordered_map<std::filesystem::path, CachedAsset<gxeng::MaterialShader>, PathHash> m_cachedMaterialShaders;
	std::unordered_map<std::filesystem::path, CachedAsset<gxeng::Material>, PathHash> m_cachedMaterials;

	gxeng::GraphicsEngine* m_graphicsEngine;
};



} // namespace inl::asset