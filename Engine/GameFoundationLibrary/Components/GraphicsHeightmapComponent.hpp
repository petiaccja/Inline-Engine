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
	std::shared_ptr<gxeng::IMesh> mesh;
	std::shared_ptr<gxeng::IMaterial> material;
	std::shared_ptr<gxeng::IImage> heightmap;
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
	   cereal::make_nvp("offset", obj.entity->GetOffset()));
}


template <class Archive>
void load(Archive& ar, GraphicsHeightmapComponent& obj) {
	Vec3 direction;
	float magnitude;
	float offset;
	ar(cereal::make_nvp("mesh", obj.meshPath),
	   cereal::make_nvp("material", obj.materialPath),
	   cereal::make_nvp("heightmap", obj.heightmapPath),
	   cereal::make_nvp("scene", obj.sceneName),
	   cereal::make_nvp("direction", direction),
	   cereal::make_nvp("magnitude", magnitude),
	   cereal::make_nvp("offset", offset));

	obj.entity->SetDirection(direction);
	obj.entity->SetMagnitude(magnitude);
	obj.entity->SetOffset(offset);

	const auto& moduleArchive = dynamic_cast<const game::ModuleArchive&>(ar);
	const auto graphicsModule = moduleArchive.GetModule<GraphicsModule>();
	obj.entity = graphicsModule->CreateHeightmapEntity();
	obj.mesh = graphicsModule->LoadMesh(obj.meshPath);
	obj.material = graphicsModule->LoadMaterial(obj.materialPath);
	obj.heightmap = graphicsModule->LoadImage(obj.heightmapPath);
	obj.entity->SetMesh(obj.mesh.get());
	obj.entity->SetMaterial(obj.material.get());
	obj.entity->SetHeightmap(obj.heightmap.get());
}


} // namespace inl::gamelib