#pragma once

#include "EntityCollection.hpp"
#include <string>

namespace inl {
namespace gxeng {


class GraphicsEngine;

class MeshEntity;
class OverlayEntity;
class TextEntity;

class DirectionalLight;


class Scene {
public:
	Scene() = default;
	Scene(std::string name);
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	virtual ~Scene();

	void SetName(std::string name);
	const std::string& GetName() const;
		
	EntityCollection<MeshEntity>& GetMeshEntities();
	const EntityCollection<MeshEntity>& GetMeshEntities() const;

	EntityCollection<OverlayEntity>& GetOverlayEntities();
	const EntityCollection<OverlayEntity>& GetOverlayEntities() const;

	EntityCollection<TextEntity>& GetTextEntities();
	const EntityCollection<TextEntity>& GetTextEntities() const;

	EntityCollection<DirectionalLight>& GetDirectionalLights();
	const EntityCollection<DirectionalLight>& GetDirectionalLights() const;

private:
	EntityCollection<MeshEntity> m_meshEntities;	
	EntityCollection<OverlayEntity> m_overlayEntities;
	EntityCollection<TextEntity> m_textEntities;
	EntityCollection<DirectionalLight> m_directionalLights;

	std::string m_name;
};



} // namespace gxeng
} // namespace inl
