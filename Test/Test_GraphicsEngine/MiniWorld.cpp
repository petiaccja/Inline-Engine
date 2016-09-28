#include "MiniWorld.hpp"


MiniWorld::MiniWorld(inl::gxeng::GraphicsEngine * graphicsEngine) {
	m_graphicsEngine = graphicsEngine;

	m_worldScene.reset(m_graphicsEngine->CreateScene("World"));

	m_cubeMesh.reset(m_graphicsEngine->CreateMesh());

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
