#include "QCWorld.hpp"

#include <AssetLibrary/Model.hpp>

#include <array>

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

QCWorld::QCWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	using namespace inl::gxeng;

	m_graphicsEngine = graphicsEngine;

	// Create scene and camera
	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));
	m_camera.reset(m_graphicsEngine->CreateCamera("WorldCam"));
	m_camera->SetTargeted(true);
	m_camera->SetTarget({ 0, 0, 0 });
	m_camera->SetPosition({ 0, -8, 3 });


	// Create terrain mesh
	{
		std::array<inl::gxeng::Vertex<Position<0>, Normal<0>, TexCoord<0>>, 4> modelVertices;
		modelVertices[0].position = { -1.f, -1.f, 0.f };
		modelVertices[1].position = { 1.f, -1.f, 0.f };
		modelVertices[2].position = { 1.f, 1.f, 0.f };
		modelVertices[3].position = { -1.f, 1.f, 0.f };
		modelVertices[0].normal = { 0, 0, 1 };
		modelVertices[1].normal = { 0, 0, 1 };
		modelVertices[2].normal = { 0, 0, 1 };
		modelVertices[3].normal = { 0, 0, 1 };
		modelVertices[0].texCoord = { 0, 0 };
		modelVertices[1].texCoord = { 1, 0 };
		modelVertices[2].texCoord = { 1, 1 };
		modelVertices[3].texCoord = { 0, 1 };
		std::array<unsigned, 6> modelIndices{
			0,1,2,
			3,2,0,
		};

		m_terrainMesh.reset(m_graphicsEngine->CreateMesh());
		m_terrainMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create QC mesh
	{
		inl::asset::Model model("qc.fbx");

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_quadcopterMesh.reset(m_graphicsEngine->CreateMesh());
		m_quadcopterMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create checker texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
		std::vector<PixelT> imgData = {
			{220, 32, 32, 255},
			{32, 220, 22, 255},
			{32, 32, 220, 255},
			{64, 64, 64, 255}
		};

		m_checkerTexture.reset(m_graphicsEngine->CreateImage());
		m_checkerTexture->SetLayout(2, 2, ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
		m_checkerTexture->Update(0, 0, 2, 2, imgData.data(), PixelT::Reader());
	}

	// Set up terrain
	m_terrainEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_terrainEntity->SetMesh(m_terrainMesh.get());
	m_terrainEntity->SetTexture(m_checkerTexture.get());
	m_terrainEntity->SetPosition({ 0,0,-1 });
	m_terrainEntity->SetRotation({ 1,0,0,0 });
	m_terrainEntity->SetScale({ 3,3,3 });
	m_worldScene->GetMeshEntities().Add(m_terrainEntity.get());

	// Set up copter
	m_quadcopterEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_quadcopterEntity->SetMesh(m_quadcopterMesh.get());
	m_quadcopterEntity->SetTexture(m_checkerTexture.get());
	m_quadcopterEntity->SetPosition({ 0,0,3 });
	m_quadcopterEntity->SetRotation({ 1,0,0,0 });
	m_quadcopterEntity->SetScale({ 1,1,1 });
	m_worldScene->GetMeshEntities().Add(m_quadcopterEntity.get());

	// Set up simulation
	m_rigidBody.SetPosition({0, 0, 1});
	m_rigidBody.SetRotation({ 1, 0, 0, 0 });

	// copter parameters
	float m = 2;
	float arm_len = 0.25;
	float Ixx = arm_len*arm_len / 2 * 0.2 * 4;
	float Iyy = Ixx;
	float Izz = arm_len*arm_len * 0.2 * 4;
	mathfu::Matrix3x3f I = {
		Ixx, 0, 0,
		0, Iyy, 0,
		0, 0, Izz };
	m_rigidBody.SetMass(m);
	m_rigidBody.SetInertia(I);
	m_rigidBody.SetGravity({ 0, 0, -9.81f });
}

void QCWorld::UpdateWorld(float elapsed) {
	// Update simulation
	mathfu::Vector4f rpm = m_rotorInfo.RPM();
	m_rotor.SetRPM(rpm);
	mathfu::Vector3f force = m_rotor.NetForce();
	mathfu::Vector3f torque = m_rotor.NetTorque();
	m_rigidBody.Update(elapsed, force, torque);

	// Move quadcopter entity
	m_quadcopterEntity->SetPosition(m_rigidBody.GetPosition());
	m_quadcopterEntity->SetRotation(m_rigidBody.GetRotation());

	// Follow copter with camera
	mathfu::Vector3f frontDir = m_rigidBody.GetRotation() * mathfu::Vector3f{ 0,1,0 };
	mathfu::Vector3f upDir = m_rigidBody.GetRotation() * mathfu::Vector3f{ 0,0,1 };
	frontDir.z() = 0;
	upDir.z() = 0;
	mathfu::Vector3f viewDir = (frontDir.LengthSquared() > upDir.LengthSquared()) ? frontDir.Normalized() : upDir.Normalized();
	m_camera->SetTarget(m_rigidBody.GetPosition());
	m_camera->SetPosition(m_rigidBody.GetPosition() - viewDir * 3 + mathfu::Vector3f{ 0,0,1.5f });
}

void QCWorld::SetAspectRatio(float ar) {
	m_camera->SetFOVAspect(60.f / 180.f * 3.1419f, ar);
}

void QCWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}




void QCWorld::TiltForward(bool set) {
	m_rotorInfo.back = set;
}
void QCWorld::TiltBackward(bool set) {
	m_rotorInfo.front = set;
}
void QCWorld::TiltRight(bool set) {
	m_rotorInfo.left = set;
}
void QCWorld::TiltLeft(bool set) {
	m_rotorInfo.right = set;
}
void QCWorld::RotateRight(bool set) {
	m_rotorInfo.rotateRight = set;
}
void QCWorld::RotateLeft(bool set) {
	m_rotorInfo.rotateLeft = set;
}
void QCWorld::Ascend(bool set) {
	m_rotorInfo.ascend = set;
}
void QCWorld::Descend(bool set) {
	m_rotorInfo.descend = set;
}
void QCWorld::IncreaseBase() {
	m_rotorInfo.baseRpm += 15.f;
}
void QCWorld::DecreaseBase() {
	m_rotorInfo.baseRpm -= 15.f;
}