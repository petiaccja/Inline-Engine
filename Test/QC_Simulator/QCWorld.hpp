#pragma once


#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/Camera.hpp>
#include <GraphicsEngine_LL/DirectionalLight.hpp>
#include "RigidBody.hpp"
#include "Rotor.hpp"
#include "PIDController.hpp"


struct ControlInfo {
	float weight = 19.62; // weight!=mass is in newtowns, not kg!
	float offsetRpm = 100;
	bool ascend = false, descend = false;
	bool front = false, back = false, left = false, right = false;
	bool rotateLeft = false, rotateRight = false;
	float heading = 0.0f;

	//           >   y   <
	//           1       2
	//             \ ^ /
	//               |       x
	//             /   \
	//           3       4
	//           >       <
	mathfu::Vector4f RPM(const Rotor& rotor) const {
		mathfu::Vector3f force, torque;
		force = { 0, 0, weight + (int)ascend - (int)descend };
		torque = {
			0.05f*((int)back - (int)front),
			0.05f*((int)right - (int)left),
			0.2f*((int)rotateLeft - (int)rotateRight)
		};
		mathfu::Vector4f rpm;
		rotor.SetTorque(force, torque, rpm);
		return rpm;
	}

	mathfu::Quaternionf Orientation() const {
		auto x = mathfu::Quaternionf::FromAngleAxis(0.35f*((int)back - (int)front), {1, 0, 0});
		auto y = mathfu::Quaternionf::FromAngleAxis(0.35f*((int)right - (int)left), { 0, 1, 0 });
		auto z = mathfu::Quaternionf::FromAngleAxis(heading, { 0, 0, 1 });
		return z*y*x;
	}
};


class QCWorld {
public:
	QCWorld(inl::gxeng::GraphicsEngine* graphicsEngine);

	void UpdateWorld(float elapsed);
	void RenderWorld(float elapsed);

	void SetAspectRatio(float ar);

	void TiltForward(bool set);
	void TiltBackward(bool set);
	void TiltRight(bool set);
	void TiltLeft(bool set);
	void RotateRight(bool set);
	void RotateLeft(bool set);
	void Ascend(bool set);
	void Descend(bool set);
	void IncreaseBase();
	void DecreaseBase();
private:
	void AddTree(mathfu::Vector3f position);
private:
	// Engine
	inl::gxeng::GraphicsEngine* m_graphicsEngine;

	// Resource
	std::unique_ptr<inl::gxeng::Mesh> m_quadcopterMesh;
	std::unique_ptr<inl::gxeng::Image> m_quadcopterTexture;
	std::unique_ptr<inl::gxeng::Mesh> m_terrainMesh;
	std::unique_ptr<inl::gxeng::Image> m_terrainTexture;
	std::unique_ptr<inl::gxeng::Mesh> m_treeMesh;
	std::unique_ptr<inl::gxeng::Image> m_treeTexture;

	std::unique_ptr<inl::gxeng::Image> m_checkerTexture;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;
	std::unique_ptr<inl::gxeng::MeshEntity> m_terrainEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_quadcopterEntity;

	inl::gxeng::DirectionalLight m_sun;

	// Scenes
	std::unique_ptr<inl::gxeng::Camera> m_camera;
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;

	// Simulation
	PIDController m_controller;
	Rotor m_rotor;
	RigidBody m_rigidBody;
	ControlInfo m_rotorInfo;
};
