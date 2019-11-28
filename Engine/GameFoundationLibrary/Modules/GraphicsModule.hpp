#include <GraphicsEngine/IGraphicsEngine.hpp>
#include "AssetLibrary/ImageCache.hpp"
#include "AssetLibrary/MaterialShaderCache.hpp"
#include "AssetLibrary/MaterialCache.hpp"
#include "AssetLibrary/GraphicsMeshCache.hpp"
#include <optional>

#undef LoadImage


namespace inl::gamelib {


class GraphicsModule {
public:
	GraphicsModule(gxeng::IGraphicsEngine& engine, std::filesystem::path assetDirectory);
	GraphicsModule(const GraphicsModule&) = delete;
	GraphicsModule(GraphicsModule&&) = default;

	std::unique_ptr<gxeng::IMeshEntity> CreateMeshEntity() const;
	std::unique_ptr<gxeng::IOverlayEntity> CreateOverlayEntity() const;
	std::unique_ptr<gxeng::ITextEntity> CreateTextEntity() const;
	std::unique_ptr<gxeng::IPerspectiveCamera> CreatePerspectiveCamera(std::string name) const;
	std::unique_ptr<gxeng::IOrthographicCamera> CreateOrthographicCamera(std::string name) const;
	std::unique_ptr<gxeng::ICamera2D> CreateCamera2D(std::string name) const;
	std::unique_ptr<gxeng::IDirectionalLight> CreateDirectionalLight() const;

	gxeng::IScene& GetOrCreateScene(std::string_view name);

	std::shared_ptr<gxeng::IMesh> LoadMesh(std::filesystem::path file) const;
	std::shared_ptr<gxeng::IMaterial> LoadMaterial(std::filesystem::path file) const;
	std::shared_ptr<gxeng::IMaterialShader> LoadMaterialShader(std::filesystem::path file) const;
	std::shared_ptr<gxeng::IImage> LoadImage(std::filesystem::path file) const;

private:
	std::optional<std::reference_wrapper<gxeng::IScene>> FindScene(std::string_view name) const;
	
private:
	gxeng::IGraphicsEngine& m_engine;
	std::vector<std::unique_ptr<gxeng::IScene>> m_scenes;
	std::unique_ptr<asset::GraphicsMeshCache> m_meshCache;
	std::unique_ptr<asset::ImageCache> m_imageCache;
	std::unique_ptr<asset::MaterialShaderCache> m_materialShaderCache;
	std::unique_ptr<asset::MaterialCache> m_materialCache;
};


} // namespace inl::gamelib