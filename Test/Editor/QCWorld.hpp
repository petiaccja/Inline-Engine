#pragma once
#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>



class QCWorld {
public:
	QCWorld(inl::gxeng::GraphicsEngine* graphicsEngine);

	void UpdateWorld(float elapsed);
	//void RenderWorld(float elapsed);

	void SetAspectRatio(float ar);

	void IWantSunsetBitches();
private:
	void AddTree(mathfu::Vector3f position);
private:
	// Engine
	inl::gxeng::GraphicsEngine* m_graphicsEngine;

	// Resource
	std::unique_ptr<inl::gxeng::Mesh> m_quadcopterMesh;
	std::unique_ptr<inl::gxeng::Image> m_quadcopterTexture;
	std::unique_ptr<inl::gxeng::Mesh> m_axesMesh;
	std::unique_ptr<inl::gxeng::Image> m_axesTexture;
	std::unique_ptr<inl::gxeng::Mesh> m_terrainMesh;
	std::unique_ptr<inl::gxeng::Image> m_terrainTexture;
	std::unique_ptr<inl::gxeng::Mesh> m_treeMesh;
	std::unique_ptr<inl::gxeng::Image> m_treeTexture;

	std::unique_ptr<inl::gxeng::Image> m_checkerTexture;

	std::unique_ptr<inl::gxeng::Material> m_treeMaterial;
	std::unique_ptr<inl::gxeng::MaterialShaderGraph> m_treeShader;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;
	std::unique_ptr<inl::gxeng::MeshEntity> m_terrainEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_quadcopterEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_axesEntity;

	inl::gxeng::DirectionalLight m_sun;

	// Scenes
	std::unique_ptr<inl::gxeng::PerspectiveCamera> m_camera;
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;
};
