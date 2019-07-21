#pragma once

#include "AssetCache.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Resources/IMaterialShader.hpp>


namespace inl::asset {


class MaterialShaderCache : public AssetCache<gxeng::IMaterialShader> {
public:
	MaterialShaderCache(gxeng::IGraphicsEngine& engine);

protected:
	std::shared_ptr<gxeng::IMaterialShader> Create(const std::filesystem::path& path) override;

private:
	gxeng::IGraphicsEngine& m_engine;
};


} // namespace inl::asset