#pragma once


#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Material.hpp>
#include <GraphicsEngine_LL/MaterialShader.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/OverlayEntity.hpp>
#include <GraphicsEngine_LL/PerspectiveCamera.hpp>
#include <GraphicsEngine_LL/Camera2D.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include <GraphicsEngine_LL/Font.hpp>
#include <GraphicsEngine_LL/TextEntity.hpp>
#include "RigidBody.hpp"
#include "Rotor.hpp"
#include "PIDController.hpp"
#include <InlineMath.hpp>


struct ControlInfo {
	float weight = 19.62f; // weight!=mass is in newtowns, not kg!
	float offsetRpm = 100.f;
	float ascend = 0.f, descend = 0.f;
	float front = 0.f, back = 0.f, left = 0.f, right = 0.f;
	float rotateLeft = 0.f, rotateRight = 0.f;
	float heading = 0.0f;

	//           >   y   <
	//           1       2
	//             \ ^ /
	//               |       x
	//             /   \
	//           3       4
	//           >       <
	inl::Vec4 RPM(const Rotor& rotor) const {
		inl::Vec3 force, torque;
		force = { 0, 0, weight + ascend - descend };
		torque = {
			0.05f*(back - front),
			0.05f*(right - left),
			0.2f*(rotateLeft - rotateRight)
		};
		inl::Vec4 rpm;
		rotor.SetTorque(force, torque, rpm);
		return rpm;
	}

	inl::Quat Orientation() const {
		auto x = inl::Quat::AxisAngle(inl::Vec3{ 1, 0, 0 }, 0.35f*(back - front));
		auto y = inl::Quat::AxisAngle(inl::Vec3{ 0, 1, 0 }, 0.35f*(right - left));
		auto z = inl::Quat::AxisAngle(inl::Vec3{ 0, 0, 1 }, heading);
		return z*y*x;
	}
};


class QCWorld {
public:
	QCWorld(inl::gxeng::GraphicsEngine* graphicsEngine);

	void UpdateWorld(float elapsed);
	void RenderWorld(float elapsed);

	void ScreenSizeChanged(int width, int height);

	void TiltForward(float set);
	void TiltBackward(float set);
	void TiltRight(float set);
	void TiltLeft(float set);
	void RotateRight(float set);
	void RotateLeft(float set);
	void Ascend(float set);
	void Descend(float set);
	void IncreaseBase();
	void DecreaseBase();
	void Heading(float set);
	float Heading() const;
	void Look(float set) { lookTilt = set; }
	float Look() const { return lookTilt; }

	void IWantSunsetBitches();
private:
	void AddTree(inl::Vec3 position);
	void CreatePipelineResources();
	std::string AssetPath(std::string name) const;
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
	std::unique_ptr<inl::gxeng::Mesh> m_sphereMesh;

	std::unique_ptr<inl::gxeng::Mesh> m_billboardMesh;

	std::unique_ptr<inl::gxeng::Image> m_sphereAlbedoTex;
	std::unique_ptr<inl::gxeng::Image> m_sphereMetalnessTex;
	std::unique_ptr<inl::gxeng::Image> m_sphereRoughnessTex;
	std::unique_ptr<inl::gxeng::Image> m_sphereNormalTex;
	std::unique_ptr<inl::gxeng::Image> m_sphereAOTex;

	std::unique_ptr<inl::gxeng::Image> m_billboardSorosTex;


	std::unique_ptr<inl::gxeng::Image> m_checkerTexture;

	std::unique_ptr<inl::gxeng::Material> m_treeMaterial;
	std::unique_ptr<inl::gxeng::Material> m_quadcopterMaterial;
	std::unique_ptr<inl::gxeng::Material> m_terrainMaterial;
	std::unique_ptr<inl::gxeng::Material> m_sphereMaterial;
	std::unique_ptr<inl::gxeng::Material> m_axesMaterial;
	std::unique_ptr<inl::gxeng::Material> m_billboardMaterial;
	std::unique_ptr<inl::gxeng::MaterialShaderGraph> m_simpleShader;
	std::unique_ptr<inl::gxeng::MaterialShaderGraph> m_pbrShader;

	std::unique_ptr<inl::gxeng::Font> m_font;

	// Pipeline resources
	std::unique_ptr<inl::gxeng::Image> m_areaImage, m_searchImage;
	std::unique_ptr<inl::gxeng::Image> m_lensFlareColorImage;
	std::unique_ptr<inl::gxeng::Image> m_colorGradingLutImage, m_lensFlareDirtImage, m_lensFlareStarImage;
	std::unique_ptr<inl::gxeng::Image> m_fontImage;
	std::unique_ptr<std::vector<char> > m_fontBinary;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;
	std::unique_ptr<inl::gxeng::MeshEntity> m_terrainEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_sphereEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_quadcopterEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_axesEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_billboardEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_billboardEntity2;

	inl::gxeng::DirectionalLight m_sun;

	// Scenes
	std::unique_ptr<inl::gxeng::PerspectiveCamera> m_camera;
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;

	// Gui
	std::unique_ptr<inl::gxeng::Camera2D> m_guiCamera;
	std::unique_ptr<inl::gxeng::Scene> m_guiScene;
	std::unique_ptr<inl::gxeng::Mesh> m_overlayQuadMesh;
	std::unique_ptr<inl::gxeng::Image> m_overlayTexture;
	std::vector<std::unique_ptr<inl::gxeng::OverlayEntity>> m_overlayElements;
	std::unique_ptr<inl::gxeng::TextEntity> m_infoText;
	std::unique_ptr<inl::gxeng::TextEntity> m_fideszText;
	bool m_textFlashing = false;

	// Simulation
	PIDController m_controller;
	Rotor m_rotor;
	RigidBody m_rigidBody;
	ControlInfo m_rotorInfo;
	float lookTilt = -0.4f;
};
