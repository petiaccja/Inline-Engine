#pragma once

#include "AssetCache.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Resources/IMesh.hpp>


namespace inl::asset {


class GraphicsMeshCache : public AssetCache<gxeng::IMesh> {
public:
	GraphicsMeshCache(gxeng::IGraphicsEngine& engine);

protected:
	std::shared_ptr<gxeng::IMesh> Create(const std::filesystem::path& path) override;

private:
	gxeng::IGraphicsEngine& m_engine;
};


} // namespace inl::asset
