#pragma once

#include "../Modules/GraphicsModule.hpp"

#include <GameLogic/AutoRegisterComponent.hpp>
#include <GraphicsEngine/Scene/IMeshEntity.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <memory>



namespace inl::gamelib {


struct GraphicsMeshComponent {
	std::unique_ptr<gxeng::IMeshEntity> entity;
	std::string meshPath;
	std::string materialPath;
	std::string sceneName;

private:
	static constexpr char ClassName[] = "GraphicsMeshComponent";
	inline static const game::AutoRegisterComponent<GraphicsMeshComponent, ClassName> reg = {};
};


template <class Archive>
void save(Archive& ar, const GraphicsMeshComponent& obj) {
	ar(cereal::make_nvp("mesh", obj.meshPath),
	   cereal::make_nvp("material", obj.materialPath),
	   cereal::make_nvp("scene", obj.sceneName));
}


template <class Archive>
void load(Archive& ar, GraphicsMeshComponent& obj) {
	ar(cereal::make_nvp("mesh", obj.meshPath),
	   cereal::make_nvp("material", obj.materialPath),
	   cereal::make_nvp("scene", obj.sceneName));
	GraphicsModule& graphicsModule = *ar.Modules().Get<std::shared_ptr<GraphicsModule>>();
	obj.entity = graphicsModule.CreateMeshEntity();
	obj.entity->SetMesh(graphicsModule.LoadMesh(obj.meshPath));
	obj.entity->SetMaterial(graphicsModule.LoadMaterial(obj.materialPath));

	graphicsModule.GetOrCreateScene(obj.sceneName).GetEntities<gxeng::IMeshEntity>().Add(obj.entity.get());
}


} // namespace inl::gamelib