#include "Node_TescoRender.hpp"

#include "../MeshEntity.hpp"
#include "../Mesh.hpp"

#include <iostream> // debug only


namespace inl {
namespace gxeng {
namespace nodes {



bool CheckMeshFormat(const Mesh& mesh) {
	return false;
}



void TescoRender::RenderScene(Texture2D * target, const EntityCollection<MeshEntity>& entities, GraphicsCommandList& commandList) {
	// Set render target
	commandList.SetResourceState(std::shared_ptr<GenericResource>(target, [](void*) {}), 0, gxapi::eResourceState::RENDER_TARGET);
	commandList.SetRenderTargets(1, &target, nullptr); // no depth yet

	// Iterate over all entities
	for (const MeshEntity* entity : entities) {
		std::cout << "Rendering entity " << entity << std::endl;
		
		// Get entity parameters
		Mesh* mesh = entity->GetMesh();
		auto position = entity->GetPosition();

		// Draw mesh
		if (!CheckMeshFormat(*mesh)) {
			std::cout << "Invalid mesh format." << std::endl;
			continue;
		}
	}
}



} // namespace nodes
} // namespace gxeng
} // namespace inl
