#pragma once


#include <GraphicsEngine_LL/GraphicsEngine.hpp>
#include <GraphicsEngine_LL/Mesh.hpp>
#include <GraphicsEngine_LL/Image.hpp>
#include <GraphicsEngine_LL/MeshEntity.hpp>
#include <GraphicsEngine_LL/Scene.hpp>
#include <GraphicsEngine_LL/Camera.hpp>
#include "RigidBody.hpp"
#include "Rotor.hpp"


struct RotorInfo {
	float baseRpm = 3763;
	float offsetRpm = 100;
	bool ascend = false, descend = false;
	bool front = false, back = false, left = false, right = false;
	bool rotateLeft = false, rotateRight = false;

	//           >   y   <
	//           1       2
	//             \ ^ /
	//               |       x
	//             /   \
	//           3       4
	//           >       <
	mathfu::Vector4f RPM() const {
		mathfu::Vector4f rpm{ baseRpm, baseRpm, baseRpm, baseRpm };
		rpm[0] += 0.6f*offsetRpm*((int)front + (int)left);
		rpm[1] += 0.6f*offsetRpm*((int)front + (int)right);
		rpm[2] += 0.6f*offsetRpm*((int)back + (int)left);
		rpm[3] += 0.6f*offsetRpm*((int)back + (int)right);

		rpm[0] += 2.0f*offsetRpm*((int)ascend - (int)descend);
		rpm[1] += 2.0f*offsetRpm*((int)ascend - (int)descend);
		rpm[2] += 2.0f*offsetRpm*((int)ascend - (int)descend);
		rpm[3] += 2.0f*offsetRpm*((int)ascend - (int)descend);

		rpm[0] += 5.0f*offsetRpm*(+(int)rotateLeft - (int)rotateRight);
		rpm[1] += 5.0f*offsetRpm*(-(int)rotateLeft + (int)rotateRight);
		rpm[2] += 5.0f*offsetRpm*(-(int)rotateLeft + (int)rotateRight);
		rpm[3] += 5.0f*offsetRpm*(+(int)rotateLeft - (int)rotateRight);
		return rpm;
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

	// Scenes
	std::unique_ptr<inl::gxeng::Scene> m_worldScene;
	std::unique_ptr<inl::gxeng::Camera> m_camera;

	// Entities
	std::vector<std::unique_ptr<inl::gxeng::MeshEntity>> m_staticEntities;
	std::unique_ptr<inl::gxeng::MeshEntity> m_terrainEntity;
	std::unique_ptr<inl::gxeng::MeshEntity> m_quadcopterEntity;

	// Simulation
	RigidBody m_rigidBody;
	Rotor m_rotor;
	RotorInfo m_rotorInfo;
};
