#pragma once


#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>



class MiniWorld {
public:
	MiniWorld(inl::gxeng::GraphicsEngine* graphicsEngine);
	
	void UpdateWorld(float elapsed);
	void RenderWorld(float elapsed);

private:
	// Engine
	inl::gxeng::GraphicsEngine* m_graphicsEngine;

	// Meshes
	std::unique_ptr<inl::gxeng::Mesh> m_cubeMesh;

	// Scenes
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;

};