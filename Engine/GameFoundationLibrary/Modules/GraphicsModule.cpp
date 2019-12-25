#include "GraphicsModule.hpp"

#include <GraphicsEngine/Scene/ICamera2D.hpp>
#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>
#include <GraphicsEngine/Scene/IOrthographicCamera.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/ITextEntity.hpp>


namespace inl::gamelib {


GraphicsModule::GraphicsModule(gxeng::IGraphicsEngine& engine, std::filesystem::path assetDirectory)
	: m_engine(engine),
	  m_meshCache(std::make_unique<asset::GraphicsMeshCache>(engine)),
	  m_imageCache(std::make_unique<asset::ImageCache>(engine)),
	  m_materialShaderCache(std::make_unique<asset::MaterialShaderCache>(engine)),
	  m_materialCache(std::make_unique<asset::MaterialCache>(engine, *m_materialShaderCache, *m_imageCache)) {
	m_meshCache->SetSearchDirectories({ assetDirectory });
	m_imageCache->SetSearchDirectories({ assetDirectory });
	m_materialShaderCache->SetSearchDirectories({ assetDirectory });
	m_materialCache->SetSearchDirectories({ assetDirectory });
}

std::unique_ptr<gxeng::IMeshEntity> GraphicsModule::CreateMeshEntity() const {
	return m_engine.CreateMeshEntity();
}

std::unique_ptr<gxeng::IOverlayEntity> GraphicsModule::CreateOverlayEntity() const {
	return m_engine.CreateOverlayEntity();
}

std::unique_ptr<gxeng::ITextEntity> GraphicsModule::CreateTextEntity() const {
	return m_engine.CreateTextEntity();
}

std::unique_ptr<gxeng::IPerspectiveCamera> GraphicsModule::CreatePerspectiveCamera(std::string name) const {
	return m_engine.CreatePerspectiveCamera(std::move(name));
}

std::unique_ptr<gxeng::IOrthographicCamera> GraphicsModule::CreateOrthographicCamera(std::string name) const {
	return m_engine.CreateOrthographicCamera(std::move(name));
}

std::unique_ptr<gxeng::ICamera2D> GraphicsModule::CreateCamera2D(std::string name) const {
	return m_engine.CreateCamera2D(std::move(name));
}

std::unique_ptr<gxeng::IDirectionalLight> GraphicsModule::CreateDirectionalLight() const {
	return m_engine.CreateDirectionalLight();
}

gxeng::IScene& GraphicsModule::GetOrCreateScene(std::string_view name) {
	auto scene = FindScene(name);
	if (!scene) {
		m_scenes.push_back(m_engine.CreateScene(std::string{ name }));
		return *m_scenes.back();
	}
	return scene.value();
}

std::shared_ptr<gxeng::IMesh> GraphicsModule::LoadMesh(std::filesystem::path file) const {
	return m_meshCache->Load(file);
}

std::shared_ptr<gxeng::IMaterial> GraphicsModule::LoadMaterial(std::filesystem::path file) const {
	return m_materialCache->Load(file);
}

std::shared_ptr<gxeng::IMaterialShader> GraphicsModule::LoadMaterialShader(std::filesystem::path file) const {
	return m_materialShaderCache->Load(file);
}

std::shared_ptr<gxeng::IImage> GraphicsModule::LoadImage(std::filesystem::path file) const {
	return m_imageCache->Load(file);
}

std::optional<std::reference_wrapper<gxeng::IScene>> GraphicsModule::FindScene(std::string_view name) const {
	for (auto& scenePtr : m_scenes) {
		if (scenePtr->GetName() == name) {
			return *scenePtr;
		}
	}
	return {};
}

} // namespace inl::gamelib
