#pragma once

#include <AssetLibrary/GraphicsMeshCache.hpp>
#include <AssetLibrary/MaterialCache.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <memory>


namespace inl::gxeng {
class IScene;
class IGraphicsEngine;
} // namespace inl::gxeng


namespace inl::gamelib {


struct GraphicsMeshProperties {
	std::string meshPath;
	std::string materialPath;
	std::string sceneName;
};


class GraphicsMeshComponentFactory : public game::ComponentClassFactoryBase {
public:
	void SetCaches(std::shared_ptr<asset::GraphicsMeshCache> meshCache, std::shared_ptr<asset::MaterialCache> materialCache);
	void SetEngine(gxeng::IGraphicsEngine* engine) noexcept;
	void SetScenes(const std::vector<gxeng::IScene*>& scenes);
	void Create(game::Entity& entity) override;
	void Create(game::Entity& entity, game::InputArchive& archive) override;

private:
	gxeng::IGraphicsEngine* m_engine = nullptr;
	std::unordered_map<std::string, gxeng::IScene*> m_scenes;
	std::shared_ptr<asset::GraphicsMeshCache> m_meshCache;
	std::shared_ptr<asset::MaterialCache> m_materialCache;
};


struct GraphicsMeshComponent {
	std::unique_ptr<gxeng::IMeshEntity> entity;
	std::shared_ptr<gxeng::IMesh> mesh;
	std::shared_ptr<gxeng::IMaterial> material;
	GraphicsMeshProperties properties;

private:
	static constexpr char ClassName[] = "GraphicsMeshComponent";
	inline static const game::AutoRegisterComponent<GraphicsMeshComponent, ClassName, GraphicsMeshComponentFactory> reg = {};
};


template <class Archive>
void serialize(Archive& ar, GraphicsMeshComponent& obj) {
	ar(cereal::make_nvp("mesh", obj.properties.meshPath),
	   cereal::make_nvp("material", obj.properties.materialPath),
	   cereal::make_nvp("scene", obj.properties.sceneName));
}



} // namespace inl::gamelib