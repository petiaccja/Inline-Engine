#pragma once

#include "../Modules/GraphicsModule.hpp"

#include <BaseLibrary/Serialization/Math.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>
#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>

#include <cereal/types/string.hpp>


namespace inl::gamelib {


struct DirectionalLightComponent {
	std::unique_ptr<gxeng::IDirectionalLight> entity;
	std::string sceneName;

private:
	static constexpr char ClassName[] = "DirectionalLightComponent";
	inline static const game::AutoRegisterComponent<DirectionalLightComponent, ClassName> reg = {};
};


template <class Archive>
void save(Archive& ar, const DirectionalLightComponent& obj) {
	ar(cereal::make_nvp("scene", obj.sceneName),
	   cereal::make_nvp("color", obj.entity->GetColor()),
	   cereal::make_nvp("direction", obj.entity->GetDirection()));
}


template <class Archive>
void load(Archive& ar, DirectionalLightComponent& obj) {
	Vec3 color;
	Vec3 direction;
	ar(cereal::make_nvp("scene", obj.sceneName),
	   cereal::make_nvp("color", color),
	   cereal::make_nvp("direction", direction));

	const auto& moduleArchive = dynamic_cast<const game::ModuleArchive&>(ar);
	const auto graphicsModule = moduleArchive.GetModule<GraphicsModule>();
	obj.entity = graphicsModule->CreateDirectionalLight();

	obj.entity->SetColor(color);
	obj.entity->SetDirection(direction);
}


} // namespace inl::gamelib