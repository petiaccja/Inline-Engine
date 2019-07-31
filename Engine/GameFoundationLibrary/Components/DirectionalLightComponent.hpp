#pragma once

#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>
#include <GraphicsEngine/Scene/IDirectionalLight.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>


namespace inl::gxeng {
class IGraphicsEngine;
} // namespace inl::gxeng


namespace inl::gamelib {


class DirectionalLightComponentFactory : public game::ComponentClassFactoryBase {
public:
	void SetEngine(gxeng::IGraphicsEngine* engine) noexcept;
	void SetScenes(const std::vector<gxeng::IScene*>& scenes);
	void Create(game::Entity& entity) override;
	void Create(game::Entity& entity, game::InputArchive& archive) override;
	std::unique_ptr<ComponentClassFactoryBase> Clone() override;

private:
	gxeng::IGraphicsEngine* m_engine = nullptr;
	std::unordered_map<std::string, gxeng::IScene*> m_scenes;
};



struct DirectionalLightComponent {
	std::unique_ptr<gxeng::IDirectionalLight> entity;

private:
	static constexpr char ClassName[] = "DirectionalLightComponent";
	inline static const game::AutoRegisterComponent<DirectionalLightComponent, ClassName, DirectionalLightComponentFactory> reg = {};
};


} // namespace inl::gamelib