#pragma once

#include <BaseLibrary/Serialization/Math.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>
#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>

#include <cereal/types/string.hpp>


namespace inl::gxeng {
class IGraphicsEngine;
} // namespace inl::gxeng


namespace inl::gamelib {


class [[deprecated("Special factories should disappear.")]] DirectionalLightComponentFactory : public game::ComponentClassFactoryBase {
public:
	void SetEngine(gxeng::IGraphicsEngine* engine) noexcept;
	void SetScenes(const std::vector<gxeng::IScene*>& scenes);
	void Create(game::Entity& entity) override;
	void Load(game::Entity& entity, game::InputArchive& archive) override;
	void Save(const game::Entity& entity, size_t componentIndex, game::OutputArchive& archive) override { throw NotImplementedException(); }
	std::unique_ptr<ComponentClassFactoryBase> Clone() override;
	const std::unordered_map<std::string, gxeng::IScene*>& GetScenes() const;
	
private:
	gxeng::IGraphicsEngine* m_engine = nullptr;
	std::unordered_map<std::string, gxeng::IScene*> m_scenes;
};


struct DirectionalLightProperties {
	std::string sceneName;
};


struct DirectionalLightComponent {
	std::unique_ptr<gxeng::IDirectionalLight> entity;
	DirectionalLightProperties properties;

private:
	static constexpr char ClassName[] = "DirectionalLightComponent";
	inline static const game::AutoRegisterComponent<DirectionalLightComponent, ClassName, DirectionalLightComponentFactory> reg = {};
};


template <class Archive>
void save(Archive& ar, const DirectionalLightComponent& obj) {
	ar(cereal::make_nvp("scene", obj.properties.sceneName),
	   cereal::make_nvp("color", obj.entity->GetColor()),
	   cereal::make_nvp("direction", obj.entity->GetDirection()));
}

template <class Archive>
void load(Archive& ar, DirectionalLightComponent& obj) {
	Vec3 color;
	Vec3 direction;
	ar(cereal::make_nvp("scene", obj.properties.sceneName),
	   cereal::make_nvp("color", color),
	   cereal::make_nvp("direction", direction));
	obj.entity->SetColor(color);
	obj.entity->SetDirection(direction);
}


} // namespace inl::gamelib