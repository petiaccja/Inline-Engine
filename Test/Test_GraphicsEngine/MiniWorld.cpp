#include "MiniWorld.hpp"

#include <array>

MiniWorld::MiniWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	m_graphicsEngine = graphicsEngine;

	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));

	{
		using namespace inl::gxeng;

		using ColoredVertexT = Vertex<Position<0>, Color<0>>;
		std::array<ColoredVertexT, 3> vertices;

		vertices[0].position = {-0.5, -0.5, 0};
		vertices[0].color = {1, 0, 0};

		vertices[1].position = {0.5, -0.5, 0};
		vertices[1].color = {0, 1, 0};

		vertices[2].position = {0, 0.5, 0};
		vertices[2].color = {0, 0, 1};

		const std::vector<unsigned> indices = {0, 1, 2};

		m_cubeMesh.reset(m_graphicsEngine->CreateMesh());
		m_cubeMesh->Set(vertices.data(), vertices.size(), sizeof(ColoredVertexT), indices.data(), indices.size());
	}

	for (int i=0; i<10; i++) {
		std::unique_ptr<inl::gxeng::MeshEntity> entity(m_graphicsEngine->CreateMeshEntity());
		entity->SetMesh(m_cubeMesh.get());
		mathfu::Vector<float, 3> pos;
		pos.x() = i - 5;
		pos.y() = 0;
		pos.z() = 0;
		entity->SetPosition(pos);

		m_worldScene->GetMeshEntities().Add(entity.get());
		m_staticEntities.push_back(std::move(entity));
	}
}

void MiniWorld::UpdateWorld(float elapsed) {
	// nothing to do
}

void MiniWorld::RenderWorld(float elapsed) {
	m_graphicsEngine->Update(elapsed);
}
