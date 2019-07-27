#include "GraphicsMeshComponent.hpp"

#include <GraphicsEngine/IGraphicsEngine.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


namespace inl::gamelib {


void GraphicsMeshComponentFactory::SetCaches(std::shared_ptr<asset::GraphicsMeshCache> meshCache, std::shared_ptr<asset::MaterialCache> materialCache) {
	m_meshCache = std::move(meshCache);
	m_materialCache = std::move(materialCache);
}


void GraphicsMeshComponentFactory::SetEngine(gxeng::IGraphicsEngine* engine) noexcept {
	m_engine = engine;
}


void GraphicsMeshComponentFactory::SetScenes(const std::vector<gxeng::IScene*>& scenes) {
	std::unordered_map<std::string, gxeng::IScene*> scenesByName;
	for (auto scene : scenes) {
		auto&& name = scene->GetName();
		auto [it, newly] = scenesByName.insert({ name, scene });
		if (!newly) {
			throw InvalidArgumentException("Multiple scene have the same name.", name);
		}
	}
	m_scenes = std::move(scenesByName);
}


void GraphicsMeshComponentFactory::Create(game::Entity& entity) {
	entity.AddComponent(GraphicsMeshComponent{});
}


void GraphicsMeshComponentFactory::Create(game::Entity& entity, game::InputArchive& archive) {
	GraphicsMeshComponent component;
	archive(component);
	if (!m_engine) {
		throw InvalidStateException("Please set graphics engine before attempting to create entities.");
	}
	auto sceneIt = m_scenes.find(component.properties.sceneName);
	if (sceneIt == m_scenes.end()) {
		throw InvalidStateException("Scene specified in the serialized entity was not found.", component.properties.sceneName);
	}
	if (!m_meshCache || !m_materialCache) {
		throw InvalidStateException("Mesh and material caches must be set.");
	}
	component.mesh = m_meshCache->Load(component.properties.meshPath);
	component.material = m_materialCache->Load(component.properties.materialPath);
	component.entity = std::unique_ptr<gxeng::IMeshEntity>(m_engine->CreateMeshEntity());
	component.entity->SetMesh(component.mesh.get());
	component.entity->SetMaterial(component.material.get());
	sceneIt->second->GetEntities<gxeng::IMeshEntity>().Add(component.entity.get());
	entity.AddComponent(std::move(component));
}


} // namespace inl::gamelib
