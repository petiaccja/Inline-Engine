#include "Node_TescoRender.hpp"

#include <iostream> // debug only


namespace inl {
namespace gxeng {
namespace nodes {



void TescoRender::RenderScene(Texture2D * target, const EntityCollection<MeshEntity>& entities, gxapi::IGraphicsCommandList * commandList) {
	for (const MeshEntity* entity : entities) {
		std::cout << "Rendering entity " << entity << std::endl;
	}
}



} // namespace nodes
} // namespace gxeng
} // namespace inl
