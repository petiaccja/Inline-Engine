#pragma once

#include "AssetCache.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Resources/IMaterial.hpp>


namespace inl::asset {

class MaterialShaderCache;
class ImageCache;


class MaterialCache : public AssetCache<gxeng::IMaterial> {
public:
	MaterialCache(gxeng::IGraphicsEngine& engine, MaterialShaderCache& shaderCache, ImageCache& imageCache);

protected:
	std::shared_ptr<gxeng::IMaterial> Create(const std::filesystem::path& path) override;

private:
	void SetMaterialParameter(gxeng::IMaterial::Parameter& param, std::string value);
	void SetMaterialParameter(gxeng::IMaterial::Parameter& param, float value);

private:
	gxeng::IGraphicsEngine& m_engine;
	MaterialShaderCache& m_shaderCache;
	ImageCache& m_imageCache;
};


} // namespace inl::asset