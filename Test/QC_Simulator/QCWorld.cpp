#include "QCWorld.hpp"

#include <AssetLibrary/Model.hpp>

#include <array>

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

QCWorld::QCWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	using namespace inl::gxeng;

	m_graphicsEngine = graphicsEngine;

	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));
	m_camera.reset(m_graphicsEngine->CreateCamera("WorldCam"));
	m_camera->SetTargeted(true);
	m_camera->SetTarget({0, 0, 0});
	m_camera->SetPosition({0, -8, 3});

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
		std::array<unsigned, 6> modelIndices {
			0,1,2,
			3,2,0,
		};

		m_terrainMesh.reset(m_graphicsEngine->CreateMesh());
		m_terrainMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	{
		inl::asset::Model model("qc.dae");

		auto modelVertices = model.GetVertices<Position<0>, Normal<0>, TexCoord<0>>(0);
		std::vector<unsigned> modelIndices = model.GetIndices(0);

		m_cubeMesh.reset(m_graphicsEngine->CreateMesh());
		m_cubeMesh->Set(modelVertices.data(), modelVertices.size(), modelIndices.data(), modelIndices.size());
	}

	{
		using PixelT = Pixel<ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR>;
		std::vector<PixelT> imgData = {
			{220, 32, 32, 255},
			{32, 220, 22, 255},
			{32, 32, 220, 255},
			{64, 64, 64, 255}
		};

		m_checker.reset(m_graphicsEngine->CreateImage());
		m_checker->SetLayout(2, 2, ePixelChannelType::INT8_NORM, 4, ePixelClass::LINEAR);
		m_checker->Update(0, 0, 2, 2, imgData.data(), PixelT::Reader());
	}

	m_terrain.reset(m_graphicsEngine->CreateMeshEntity());
	m_terrain->SetMesh(m_terrainMesh.get());
	m_terrain->SetTexture(m_checker.get());
	m_terrain->SetPosition({ 0,0,-1 });
	m_terrain->SetRotation({ 1,0,0,0 });
	m_terrain->SetScale({ 3,3,3 });
	m_worldScene->GetMeshEntities().Add(m_terrain.get());

	srand(time(nullptr));

	const float extent = 5;
	const int count = 6;
	for (int i = 0; i < count; i++) {
		std::unique_ptr<inl::gxeng::MeshEntity> entity(m_graphicsEngine->CreateMeshEntity());
		entity->SetMesh(m_cubeMesh.get());
		entity->SetTexture(m_checker.get());
		mathfu::Vector<float, 3> pos;
		pos.x() = float((i + 0.5f)*extent) / count - extent*0.5f;
		pos.y() = 0;
		pos.z() = 0;
		entity->SetPosition(pos);

		m_worldScene->GetMeshEntities().Add(entity.get());
		m_staticEntities.push_back(std::move(entity));
		m_velocities.push_back(mathfu::Vector<float, 3>(rand2(), rand2(), 0));
	}
}

void QCWorld::UpdateWorld(float elapsed) {
	const float boundary = 3;
	assert(m_staticEntities.size() == m_velocities.size());
	for (int i = 0; i < m_staticEntities.size(); i++) {
		auto& currEntity = m_staticEntities[i];
		auto& currVel = m_velocities[i];

		currVel += mathfu::Vector<float, 3>(rand2(), rand2(), 0)*5.f*elapsed;

		const float maxSpeed = 2.5f;
		if (currVel.Length() > maxSpeed) {
			currVel = currVel.Normalized() * maxSpeed;
		}

		auto newPos = currEntity->GetPosition() + currVel*elapsed;

		if (newPos.x() > boundary || newPos.x() < -boundary) {
			currVel.x() *= -1;
			newPos = currEntity->GetPosition();
		}
		if (newPos.y() > boundary || newPos.y() < -boundary) {
			currVel.y() *= -1;
			newPos = currEntity->GetPosition();
		}

		currEntity->SetPosition(newPos);
		currEntity->SetRotation(currEntity->GetRotation() * mathfu::Quaternion<float>::FromAngleAxis(1.5f*elapsed, currVel.Normalized()));
	}
}

void QCWorld::SetAspectRatio(float ar) {
	m_camera->SetFOVAspect(60.f / 180.f * 3.1419f, ar);
}

void QCWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}
