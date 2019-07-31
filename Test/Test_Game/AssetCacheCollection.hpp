#pragma once

#include <AssetLibrary/GraphicsMeshCache.hpp>
#include <AssetLibrary/ImageCache.hpp>
#include <AssetLibrary/MaterialCache.hpp>
#include <AssetLibrary/MaterialShaderCache.hpp>


class AssetCacheCollection {
public:
	AssetCacheCollection(inl::gxeng::IGraphicsEngine& graphicsEngine);

	inl::asset::GraphicsMeshCache& GetGraphicsMeshCache() const;
	inl::asset::MaterialCache& GetMaterialCache() const;
	inl::asset::MaterialShaderCache& GetCache() const;
	inl::asset::ImageCache& GetImageCache() const;

private:
	std::unique_ptr<inl::asset::GraphicsMeshCache> m_meshCache;
	std::unique_ptr<inl::asset::MaterialCache> m_materialCache;
	std::unique_ptr<inl::asset::MaterialShaderCache> m_materialShaderCache;
	std::unique_ptr<inl::asset::ImageCache> m_imageCache;
};
