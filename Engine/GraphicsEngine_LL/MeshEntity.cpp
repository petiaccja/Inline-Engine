#include "MeshEntity.hpp"

namespace inl::gxeng {


MeshEntity::MeshEntity() :
	m_mesh(nullptr),
	m_material(nullptr)
{}



void MeshEntity::SetMesh(Mesh* mesh) {
	m_mesh = mesh;
}
Mesh* MeshEntity::GetMesh() const {
	return m_mesh;
}

void MeshEntity::SetMaterial(Material* material) {
	m_material = material;
}
Material* MeshEntity::GetMaterial() const {
	return m_material;
}









} // namespace inl::gxeng
