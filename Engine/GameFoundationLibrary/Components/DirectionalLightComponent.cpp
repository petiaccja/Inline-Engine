#include "DirectionalLightComponent.hpp"
#include <GraphicsEngine/IGraphicsEngine.hpp> 

namespace inl::gamelib {


void DirectionalLightComponentFactory::SetEngine(gxeng::IGraphicsEngine* engine) noexcept {
	m_engine = engine;
}


void DirectionalLightComponentFactory::SetScenes(const std::vector<gxeng::IScene*>& scenes) {
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

void DirectionalLightComponentFactory::Create(game::Entity& entity) {
	DirectionalLightComponent component;
	component.entity = m_engine->CreateDirectionalLight();
	entity.AddComponent(std::move(component));
}


void DirectionalLightComponentFactory::Load(game::Entity& entity, game::InputArchive& archive) {
	if (!m_engine) {
		throw InvalidStateException("Please set graphics engine before attempting to create entities.");
	}

	DirectionalLightComponent component;
	component.entity = m_engine->CreateDirectionalLight();
	archive(component);
	entity.AddComponent(std::move(component));
}


std::unique_ptr<game::ComponentClassFactoryBase> DirectionalLightComponentFactory::Clone() {
	return std::make_unique<DirectionalLightComponentFactory>(*this);
}


const std::unordered_map<std::string, gxeng::IScene*>& DirectionalLightComponentFactory::GetScenes() const {
	return m_scenes;
}


} // namespace inl::gamelib
