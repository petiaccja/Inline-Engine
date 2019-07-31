#include "AssetCacheCollection.hpp"


AssetCacheCollection::AssetCacheCollection(inl::gxeng::IGraphicsEngine& graphicsEngine) {
	m_meshCache = std::make_unique<inl::asset::GraphicsMeshCache>(graphicsEngine);
	m_imageCache = std::make_unique<inl::asset::ImageCache>(graphicsEngine);
	m_materialShaderCache = std::make_unique<inl::asset::MaterialShaderCache>(graphicsEngine);
	m_materialCache = std::make_unique<inl::asset::MaterialCache>(graphicsEngine, *m_materialShaderCache, *m_imageCache);
	m_meshCache->SetSearchDirectories({ INL_GAMEDATA });
	m_imageCache->SetSearchDirectories({ INL_GAMEDATA });
	m_materialShaderCache->SetSearchDirectories({ INL_GAMEDATA });
	m_materialCache->SetSearchDirectories({ INL_GAMEDATA });
}


inl::asset::GraphicsMeshCache& AssetCacheCollection::GetGraphicsMeshCache() const {
	return *m_meshCache;
}


inl::asset::MaterialCache& AssetCacheCollection::GetMaterialCache() const {
	return *m_materialCache;
}


inl::asset::MaterialShaderCache& AssetCacheCollection::GetCache() const {
	return *m_materialShaderCache;
}


inl::asset::ImageCache& AssetCacheCollection::GetImageCache() const {
	return *m_imageCache;
}