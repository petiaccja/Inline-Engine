#pragma once
#include "WorldComponent.hpp"


namespace graphics { class IEntity; }

class MeshComponent : public WorldComponent
{
public:
	static const eWorldComponentType TYPE = MESH;

public:
	inline MeshComponent(graphics::IEntity* e);

	void SetTextureNormal(const std::string& contentPath);
	void SetTextureBaseColor(const std::string& contentPath);
	void SetTextureAO(const std::string& contentPath);

	inline graphics::IEntity* GetEntity();

protected:
	graphics::IEntity* entity;
};


MeshComponent::MeshComponent(graphics::IEntity* e)
:WorldComponent(TYPE), entity(e)
{
}

graphics::IEntity* MeshComponent::GetEntity()
{
	return entity;
}