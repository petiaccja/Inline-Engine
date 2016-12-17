#include "QCWorld.hpp"

#include <AssetLibrary/Model.hpp>
#include "AssetLibrary/Image.hpp"

#include <array>
#include <random>

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

QCWorld::QCWorld(inl::gxeng::GraphicsEngine* graphicsEngine) {
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
		inl::asset::Model model("assets\\terrain.fbx");

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_terrainMesh.reset(m_graphicsEngine->CreateMesh());
		m_terrainMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create terrain texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\terrain.jpg");

		m_terrainTexture.reset(m_graphicsEngine->CreateImage());
		m_terrainTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_terrainTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());
	}

	// Create QC mesh
	{
		inl::asset::Model model("assets\\quadcopter.fbx");

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_quadcopterMesh.reset(m_graphicsEngine->CreateMesh());
		m_quadcopterMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create QC texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\quadcopter.jpg");

		m_quadcopterTexture.reset(m_graphicsEngine->CreateImage());
		m_quadcopterTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_quadcopterTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());
	}

	// Create tree mesh
	{
		inl::asset::Model model("assets\\pine_tree.fbx");

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_treeMesh.reset(m_graphicsEngine->CreateMesh());
		m_treeMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	// Create tree texture
	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR>;
		inl::asset::Image img("assets\\pine_tree.jpg");

		m_treeTexture.reset(m_graphicsEngine->CreateImage());
		m_treeTexture->SetLayout(img.GetWidth(), img.GetHeight(), ePixelChannelType::INT8_NORM, 3, ePixelClass::LINEAR);
		m_treeTexture->Update(0, 0, img.GetWidth(), img.GetHeight(), img.GetData(), PixelT::Reader());
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
	m_terrainEntity->SetTexture(m_terrainTexture.get());
	m_terrainEntity->SetPosition({ 0,0,0 });
	m_terrainEntity->SetRotation({ 1,0,0,0 });
	m_terrainEntity->SetScale({ 1,1,1 });
	m_worldScene->GetMeshEntities().Add(m_terrainEntity.get());

	// Set up copter
	m_quadcopterEntity.reset(m_graphicsEngine->CreateMeshEntity());
	m_quadcopterEntity->SetMesh(m_quadcopterMesh.get());
	m_quadcopterEntity->SetTexture(m_quadcopterTexture.get());
	m_quadcopterEntity->SetPosition({ 0,0,3 });
	m_quadcopterEntity->SetRotation({ 1,0,0,0 });
	m_quadcopterEntity->SetScale({ 1,1,1 });
	m_worldScene->GetMeshEntities().Add(m_quadcopterEntity.get());

	// Set up trees
	AddTree({ 2, 2, 0 });
	AddTree({ 11, 6, 0 });
	AddTree({ 13, 8, 0 });
	AddTree({ 10.5f, 8.5f, 0 });
	AddTree({ -17, -12, 0 });
	AddTree({ 8, 14, 0 });
	AddTree({ 9, -12, 0 });

	// Set up simulation
	m_rigidBody.SetPosition({0, 0, 1});
	m_rigidBody.SetRotation({ 1, 0, 0, 0 });

	// copter parameters
	float m = 2;
	float Ixx = 0.026;
	float Iyy = 0.024;
	float Izz = 0.048;
	mathfu::Matrix3x3f I = {
		Ixx, 0, 0,
		0, Iyy, 0,
		0, 0, Izz };
	m_rigidBody.SetMass(m);
	m_rigidBody.SetInertia(I);
	m_rigidBody.SetGravity({ 0, 0, -9.81f });
	m_controller.SetInertia(I);
}

void QCWorld::UpdateWorld(float elapsed) {
	// Update QC heading
	m_rotorInfo.heading += 2.0f*((int)m_rotorInfo.rotateLeft - (int)m_rotorInfo.rotateRight)*elapsed;

	// Update simulation
	bool controller = true;
	if (!controller) {
		mathfu::Vector4f rpm = m_rotorInfo.RPM(m_rotor);
		mathfu::Vector3f force;
		mathfu::Vector3f torque;
		m_rotor.SetRPM(rpm, force, torque);
		m_rigidBody.Update(elapsed, force, torque);
	}
	else {
		mathfu::Quaternionf orientation = m_rotorInfo.Orientation();
		mathfu::Quaternionf q = m_rigidBody.GetRotation();
		mathfu::Vector3f force;
		mathfu::Vector3f torque;
		mathfu::Vector4f rpm;
		float lift = 2.0f * 9.81 + 5.f*((int)m_rotorInfo.ascend - (int)m_rotorInfo.descend);
		m_controller.Update(orientation, lift, q, m_rigidBody.GetAngularVelocity(), elapsed, force, torque);
		m_rotor.SetTorque(force, torque, rpm);
		m_rotor.SetRPM(rpm, force, torque);
		m_rigidBody.Update(elapsed, force, torque);
	}

	// Move quadcopter entity
	m_quadcopterEntity->SetPosition(m_rigidBody.GetPosition());
	m_quadcopterEntity->SetRotation(m_rigidBody.GetRotation());

	// Follow copter with camera
	mathfu::Vector3f frontDir = m_rigidBody.GetRotation() * mathfu::Vector3f{ 0,1,0 };
	mathfu::Vector3f upDir = m_rigidBody.GetRotation() * mathfu::Vector3f{ 0,0,1 };
	frontDir.z() = 0;
	upDir.z() = 0;
	mathfu::Vector3f viewDir = (5*frontDir.LengthSquared() > upDir.LengthSquared()) ? frontDir.Normalized() : upDir.Normalized();
	m_camera->SetTarget(m_rigidBody.GetPosition());
	m_camera->SetPosition(m_rigidBody.GetPosition() - viewDir * 1.5 + mathfu::Vector3f{ 0,0,0.35f });
}

void QCWorld::SetAspectRatio(float ar) {
	m_camera->SetFOVAspect(75.f / 180.f * 3.1419f, ar);
}

void QCWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}


void QCWorld::AddTree(mathfu::Vector3f position) {
	std::unique_ptr<inl::gxeng::MeshEntity> tree;

	static std::mt19937_64 rne;
	static std::uniform_real_distribution<float> rng{ 0.8f, 1.2f };
	float s = rng(rne);

	tree.reset(m_graphicsEngine->CreateMeshEntity());
	tree->SetMesh(m_treeMesh.get());
	tree->SetTexture(m_treeTexture.get());
	tree->SetPosition(position);
	tree->SetRotation({ 1,0,0,0 });
	tree->SetScale({ s,s,s });
	m_worldScene->GetMeshEntities().Add(tree.get());
	m_staticEntities.push_back(std::move(tree));
}




void QCWorld::TiltForward(bool set) {
	m_rotorInfo.front = set;
}
void QCWorld::TiltBackward(bool set) {
	m_rotorInfo.back = set;
}
void QCWorld::TiltRight(bool set) {
	m_rotorInfo.right = set;
}
void QCWorld::TiltLeft(bool set) {
	m_rotorInfo.left = set;
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
	//m_rotorInfo.baseRpm += 15.f;
}
void QCWorld::DecreaseBase() {
	//m_rotorInfo.baseRpm -= 15.f;
}