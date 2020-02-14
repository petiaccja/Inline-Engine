#include "MaterialShaderCache.hpp"

#include <fstream>


namespace inl::asset {


MaterialShaderCache::MaterialShaderCache(gxeng::IGraphicsEngine& engine)
	: m_engine(engine) {}


std::shared_ptr<gxeng::IMaterialShader> MaterialShaderCache::Create(const std::filesystem::path& path) {
	std::shared_ptr<gxeng::IMaterialShaderGraph> resource(m_engine.CreateMaterialShaderGraph());
	Reload(*resource, path);
	return resource;
}

void MaterialShaderCache::Reload(gxeng::IMaterialShader& asset, const std::filesystem::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw FileNotFoundException("Asset file exists but cannot be opened.", path.generic_string());
	}
	std::string desc((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	dynamic_cast<gxeng::IMaterialShaderGraph&>(asset).SetGraph(desc);
}


} // namespace inl::asset
