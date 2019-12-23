#pragma once

#include "../Modules/GraphicsModule.hpp"

#include <BaseLibrary/Serialization/Math.hpp>
#include <GameLogic/AutoRegisterComponent.hpp>
#include <GameLogic/ComponentClassFactory.hpp>
#include <GraphicsEngine/Scene/IPerspectiveCamera.hpp>
#include <GraphicsEngine/Scene/IScene.hpp>

#include <cereal/types/string.hpp>


namespace inl::gamelib {



struct PerspectiveCameraComponent {
	std::unique_ptr<gxeng::IPerspectiveCamera> entity;

private:
	static constexpr char ClassName[] = "PerspectiveCameraComponent";
	inline static const game::AutoRegisterComponent<PerspectiveCameraComponent, ClassName> reg = {};
};


template <class Archive>
void save(Archive& ar, const PerspectiveCameraComponent& obj) {
	ar(cereal::make_nvp("name", obj.entity->GetName()),
	   cereal::make_nvp("aspectRatio", obj.entity->GetAspectRatio()),
	   cereal::make_nvp("fieldOfView", obj.entity->GetFOVHorizontal()),
	   cereal::make_nvp("nearPlane", obj.entity->GetNearPlane()),
	   cereal::make_nvp("farPlane", obj.entity->GetFarPlane()),
	   cereal::make_nvp("position", obj.entity->GetPosition()),
	   cereal::make_nvp("target", obj.entity->GetTarget()),
	   cereal::make_nvp("upVector", obj.entity->GetUpVector()));
}


template <class Archive>
void load(Archive& ar, PerspectiveCameraComponent& obj) {
	std::string name;
	float aspectRatio;
	float fieldOfView;
	float nearPlane;
	float farPlane;
	Vec3 position;
	Vec3 target;
	Vec3 upVector;

	ar(cereal::make_nvp("name", name),
	   cereal::make_nvp("aspectRatio", aspectRatio),
	   cereal::make_nvp("fieldOfView", fieldOfView),
	   cereal::make_nvp("nearPlane", nearPlane),
	   cereal::make_nvp("farPlane", farPlane),
	   cereal::make_nvp("position", position),
	   cereal::make_nvp("target", target),
	   cereal::make_nvp("upVector", upVector));

	const auto& moduleArchive = dynamic_cast<const game::ModuleArchive&>(ar);
	const auto graphicsModule = moduleArchive.GetModule<GraphicsModule>();
	obj.entity = graphicsModule->CreatePerspectiveCamera(name);

	obj.entity->SetFOVAspect(fieldOfView, aspectRatio);
	obj.entity->SetNearPlane(nearPlane);
	obj.entity->SetFarPlane(farPlane);
	obj.entity->SetPosition(position);
	obj.entity->SetTarget(target);
	obj.entity->SetUpVector(upVector);
}


} // namespace inl::gamelib