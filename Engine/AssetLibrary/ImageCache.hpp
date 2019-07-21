#pragma once

#include "AssetCache.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Resources/IImage.hpp>


namespace inl::asset {


class ImageCache : public AssetCache<gxeng::IImage> {
public:
	ImageCache(gxeng::IGraphicsEngine& engine);

protected:
	std::shared_ptr<gxeng::IImage> Create(const std::filesystem::path& path) override;

private:
	gxeng::IGraphicsEngine& m_engine;
};


} // namespace inl::asset
