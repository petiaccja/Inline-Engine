#include "MiniWorld.hpp"

#include <array>

inline float rand2() {
	return (rand() / float(RAND_MAX)) * 2 - 1;
}

static int indices[] = {
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

MiniWorld::MiniWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	m_graphicsEngine = graphicsEngine;

	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));

	{
		using namespace inl::gxeng;

		using ColoredVertexT = Vertex<Position<0>, Color<0>>;
		std::array<ColoredVertexT, 3> vertices;

		vertices[0].position = {-0.1f, -0.1f, 0.1f};
		vertices[0].color = {1, 0, 0};

		vertices[1].position = { 0.1f, -0.1f, 0.1f };
		vertices[1].color = {0, 1, 0};

		vertices[2].position = {0.0f, 0.1f, 0.1f};
		vertices[2].color = {0, 0, 1};

		const std::vector<unsigned> indices = {0, 1, 2};

		m_cubeMesh.reset(m_graphicsEngine->CreateMesh());
		m_cubeMesh->Set(vertices.data(), vertices.size(), sizeof(ColoredVertexT), indices.data(), indices.size());
	}

	int count = 10;
	for (int i=0; i<count; i++) {
		std::unique_ptr<inl::gxeng::MeshEntity> entity(m_graphicsEngine->CreateMeshEntity());
		entity->SetMesh(m_cubeMesh.get());
		mathfu::Vector<float, 3> pos;
		pos.x() = float((i+0.5f)*2)/count - 1.f;
		pos.y() = 0;
		pos.z() = 0;
		entity->SetPosition(pos);

		m_worldScene->GetMeshEntities().Add(entity.get());
		m_staticEntities.push_back(std::move(entity));
		m_velocities.push_back(mathfu::Vector<float, 3>(rand2(), rand2(), 0)*0.5f);
	}
}

void MiniWorld::UpdateWorld(float elapsed) {
	// nothing to do
	assert(m_staticEntities.size() == m_velocities.size());
	for (int i = 0; i < m_staticEntities.size(); i++) {
		auto& currEntity = m_staticEntities[i];
		auto& currVel = m_velocities[i]; 
		
		auto newPos = currEntity->GetPosition() + currVel*elapsed;
		
		if (newPos.x() > 1 || newPos.x() < -1) {
			currVel.x() *= -1;
			newPos = currEntity->GetPosition();
		}
		if (newPos.y() > 1 || newPos.y() < -1) {
			currVel.y() *= -1;
			newPos = currEntity->GetPosition();
		}

		currEntity->SetPosition(newPos);
	}
}

void MiniWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}
