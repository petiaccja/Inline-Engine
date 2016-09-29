#include "MeshEntity.hpp"

namespace inl {
namespace gxeng {


MeshEntity::MeshEntity() {
	m_mesh = nullptr;
}



void MeshEntity::SetMesh(Mesh* mesh) {
	m_mesh = mesh;
}
Mesh* MeshEntity::GetMesh() const {
	return m_mesh;
}

void MeshEntity::SetPosition(mathfu::Vector<float, 3> pos) {
	m_position = pos;
}
mathfu::Vector<float, 3> MeshEntity::GetPosition() const {
	return m_position;
}


}
}
