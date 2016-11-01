#include "MiniWorld.hpp"

#include <array>

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

MiniWorld::MiniWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	m_graphicsEngine = graphicsEngine;

	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));

	{
		using namespace inl::gxeng;

		using ColoredVertexT = Vertex<Position<0>, Normal<0>, TexCoord<0>>;
		std::array<ColoredVertexT, 24> vertices;

		// top
		vertices[0].position = { 0.5, 0.5, 0.5 };
		vertices[0].normal = { 0, 0, 1 };
		vertices[0].texCoord = { 1, 0 };
		vertices[1].position = { -0.5, 0.5, 0.5 };
		vertices[1].normal = { 0, 0, 1 };
		vertices[1].texCoord = { 1, 1 };
		vertices[2].position = { -0.5, -0.5, 0.5 };
		vertices[2].normal = { 0, 0, 1 };
		vertices[2].texCoord = { 0, 1 };
		vertices[3].position = { 0.5, -0.5, 0.5 };
		vertices[3].normal = { 0, 0, 1 };

		// bottom
		vertices[3].texCoord = { 0, 0 };
		vertices[4].position = { 0.5, 0.5, -0.5 };
		vertices[4].normal = { 0, 0, -1 };
		vertices[4].texCoord = { 1, 0 };
		vertices[5].position = { -0.5, 0.5, -0.5 };
		vertices[5].normal = { 0, 0, -1 };
		vertices[5].texCoord = { 1, 1 };
		vertices[6].position = { -0.5, -0.5, -0.5 };
		vertices[6].normal = { 0, 0, -1 };
		vertices[6].texCoord = { 0, 1 };
		vertices[7].position = { 0.5, -0.5, -0.5 };
		vertices[7].normal = { 0, 0, -1 };
		vertices[7].texCoord = { 0, 0 };

		// right
		vertices[8].position = { 0.5, 0.5, 0.5 };
		vertices[8].normal = { 1, 0, 0 };
		vertices[8].texCoord = { 1, 0 };
		vertices[9].position = { 0.5, -0.5, 0.5 };
		vertices[9].normal = { 1, 0, 0 };
		vertices[9].texCoord = { 1, 1 };
		vertices[10].position = { 0.5, -0.5, -0.5 };
		vertices[10].normal = { 1, 0, 0 };
		vertices[10].texCoord = { 0, 1 };
		vertices[11].position = { 0.5, 0.5, -0.5 };
		vertices[11].normal = { 1, 0, 0 };
		vertices[11].texCoord = { 0, 0 };

		//left
		vertices[12].position = { -0.5, 0.5, 0.5 };
		vertices[12].normal = { -1, 0, 0 };
		vertices[12].texCoord = { 1, 0 };
		vertices[13].position = { -0.5, -0.5, 0.5 };
		vertices[13].normal = { -1, 0, 0 };
		vertices[13].texCoord = { 1, 1 };
		vertices[14].position = { -0.5, -0.5, -0.5 };
		vertices[14].normal = { -1, 0, 0 };
		vertices[14].texCoord = { 0, 1 };
		vertices[15].position = { -0.5, 0.5, -0.5 };
		vertices[15].normal = { -1, 0, 0 };
		vertices[15].texCoord = { 0, 0 };

		// front
		vertices[16].position = { 0.5, 0.5, 0.5 };
		vertices[16].normal = { 0, 1, 0 };
		vertices[16].texCoord = { 1, 0 };
		vertices[17].position = { 0.5, 0.5, -0.5 };
		vertices[17].normal = { 0, 1, 0 };
		vertices[17].texCoord = { 1, 1 };
		vertices[18].position = { -0.5, 0.5, -0.5 };
		vertices[18].normal = { 0, 1, 0 };
		vertices[18].texCoord = { 0, 1 };
		vertices[19].position = { -0.5, 0.5, 0.5 };
		vertices[19].normal = { 0, 1, 0 };
		vertices[19].texCoord = { 0, 0 };

		//back
		vertices[20].position = { 0.5, -0.5, 0.5 };
		vertices[20].normal = { 0, -1, 0 };
		vertices[20].texCoord = { 1, 0 };
		vertices[21].position = { 0.5, -0.5, -0.5 };
		vertices[21].normal = { 0, -1, 0 };
		vertices[21].texCoord = { 1, 1 };
		vertices[22].position = { -0.5, -0.5, -0.5 };
		vertices[22].normal = { 0, -1, 0 };
		vertices[22].texCoord = { 0, 1 };
		vertices[23].position = { -0.5, -0.5, 0.5 };
		vertices[23].normal = { 0, -1, 0 };
		vertices[23].texCoord = { 0, 0 };

		const std::vector<unsigned> indices = {
			// top
			0,1,3,
			1,2,3,
			// bottom
			5,4,7,
			6,5,7,
			// right
			8,9,11,
			9,10,11,
			// left
			13,12,15,
			14,13,15,
			// front
			16,17,19,
			17,18,19,
			// back
			21,20,23,
			22,21,23,
		};

		m_cubeMesh.reset(m_graphicsEngine->CreateMesh());
		m_cubeMesh->Set(vertices.data(), vertices.size(), sizeof(ColoredVertexT), indices.data(), indices.size());
	}

	inl::gxeng::Texture2DSRV texture;
	{
		std::vector<std::array<float, 4>> imgData = {
			{9.0f, 0.2f, 0.2f, 1.0f},
			{0.2f, 9.0f, 0.2f, 1.0f},
			{0.2f, 0.2f, 9.0f, 1.0f},
			{0.3f, 0.3f, 0.3f, 1.0f}
		};

		texture = m_graphicsEngine->DEBUG_CreateTexture(imgData.data(), 2, 2, inl::gxapi::eFormat::R32G32B32A32_FLOAT);
	}

	const float extent = 3;
	const int count = 6;
	for (int i=0; i<count; i++) {
		std::unique_ptr<inl::gxeng::DEBUG_TexturedEntity> entity(new inl::gxeng::DEBUG_TexturedEntity());
		entity->SetMesh(m_cubeMesh.get());
		entity->SetTexture(texture);
		mathfu::Vector<float, 3> pos;
		pos.x() = float((i+0.5f)*extent)/count - extent*0.5f;
		pos.y() = 0;
		pos.z() = 0;
		entity->SetPosition(pos);

		m_worldScene->GetMeshEntities().Add(entity.get());
		m_staticEntities.push_back(std::move(entity));
		m_velocities.push_back(mathfu::Vector<float, 3>(rand2(), rand2(), 0));
	}
}

void MiniWorld::UpdateWorld(float elapsed) {
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

void MiniWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}
