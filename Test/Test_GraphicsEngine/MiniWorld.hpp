#pragma once


#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/Camera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>


class MiniWorld {
public:
	MiniWorld(inl::gxeng::GraphicsEngine* graphicsEngine);
	
	void UpdateWorld(float elapsed);
	void RenderWorld(float elapsed);

	void SetAspectRatio(float ar);
private:
	// Engine
	inl::gxeng::GraphicsEngine* m_graphicsEngine;

	// Resource
	std::unique_ptr<inl::gxeng::Mesh> m_cubeMesh;
	std::unique_ptr<inl::gxeng::Image> m_texture;

	// Scenes
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;
	std::unique_ptr<inl::gxeng::Camera> m_camera;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;
	std::vector<mathfu::Vector<float, 3>> m_velocities;
	inl::gxeng::DirectionalLight m_sun;
};
