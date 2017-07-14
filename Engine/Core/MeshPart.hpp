#pragma once
#include "Part.hpp"
#include <GraphicsEngine_LL\MeshEntity.hpp>

namespace inl::core {

class MeshPart : public Part
{
public:
	static const ePartType TYPE = MESH;

public:
	MeshPart(gxeng::MeshEntity* e);

	void SetTextureNormal(const std::string& contentPath);
	void SetTextureBaseColor(const std::string& contentPath);
	void SetTextureAO(const std::string& contentPath);

	gxeng::MeshEntity* GetEntity();

protected:
	gxeng::MeshEntity* entity;
};

} // namespace inl::core