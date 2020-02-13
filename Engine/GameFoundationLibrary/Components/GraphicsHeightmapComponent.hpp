#pragma once

#include "../Modules/GraphicsModule.hpp"

#include "GraphicsEngine/Scene/IHeightmapEntity.hpp"
#include <GameLogic/AutoRegisterComponent.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <memory>


namespace inl::gamelib {


struct GraphicsHeightmapComponent {
	std::unique_ptr<gxeng::IHeightmapEntity> entity;
	std::string meshPath;
	std::string materialPath;
	std::string heightmapPath;
	std::string sceneName;

private:
	static constexpr char ClassName[] = "GraphicsHeightmapComponent";
	inline static const game::AutoRegisterComponent<GraphicsHeightmapComponent, ClassName> reg = {};
};


template <class Archive>
void save(Archive& ar, const GraphicsHeightmapComponent& obj) {
	ar(cereal::make_nvp("mesh", obj.meshPath),
	   cereal::make_nvp("material", obj.materialPath),
	   cereal::make_nvp("heightmap", obj.heightmapPath),
	   cereal::make_nvp("scene", obj.sceneName),
	   cereal::make_nvp("direction", obj.entity->GetDirection()),
	   cereal::make_nvp("magnitude", obj.entity->GetMagnitude()),
	   cereal::make_nvp("offset", obj.entity->GetOffset()),
	   cereal::make_nvp("uvsize", obj.entity->GetUvSize()));
}


template <class Archive>
void load(Archive& ar, GraphicsHeightmapComponent& obj) {
	Vec3 direction;
	float magnitude;
	float offset;
	Vec2 uvSize;
	ar(cereal::make_nvp("mesh", obj.meshPath),
	   cereal::make_nvp("material", obj.materialPath),
	   cereal::make_nvp("heightmap", obj.heightmapPath),
	   cereal::make_nvp("scene", obj.sceneName),
	   cereal::make_nvp("direction", direction),
	   cereal::make_nvp("magnitude", magnitude),
	   cereal::make_nvp("offset", offset),
	   cereal::make_nvp("uvsize", uvSize));

	GraphicsModule& graphicsModule = *ar.Modules().Get<std::shared_ptr<GraphicsModule>>();
	obj.entity = graphicsModule.CreateHeightmapEntity();
	obj.entity->SetMesh(graphicsModule.LoadMesh(obj.meshPath));
	obj.entity->SetMaterial(graphicsModule.LoadMaterial(obj.materialPath));
	obj.entity->SetHeightmap(graphicsModule.LoadImage(obj.heightmapPath));

	obj.entity->SetDirection(direction);
	obj.entity->SetMagnitude(magnitude);
	obj.entity->SetOffset(offset);
	obj.entity->SetUvSize(uvSize);

	graphicsModule.GetOrCreateScene(obj.sceneName).GetEntities<gxeng::IHeightmapEntity>().Add(obj.entity.get());
}


} // namespace inl::gamelib