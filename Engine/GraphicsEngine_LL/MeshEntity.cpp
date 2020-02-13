#include "MeshEntity.hpp"
#include "GuiEngine/Board.hpp"
#include "GuiEngine/Board.hpp"
#include "GuiEngine/Board.hpp"
#include "GuiEngine/Board.hpp"

namespace inl::gxeng {



void MeshEntity::SetMesh(std::shared_ptr<Mesh> mesh) {
	m_mesh = mesh;
}

std::shared_ptr<IMesh> MeshEntity::GetMesh() const {
	return m_mesh;
}

const std::shared_ptr<Mesh>& MeshEntity::GetMeshNative() const {
	return m_mesh;
}

void MeshEntity::SetMaterial(std::shared_ptr<Material> material) {
	m_material = material;
}

std::shared_ptr<IMaterial> MeshEntity::GetMaterial() const {
	return m_material;
}

const std::shared_ptr<Material>& MeshEntity::GetMaterialNative() const {
	return m_material;
}

Transform3D& MeshEntity::Transform() {
	return m_transform;
}

const Transform3D& MeshEntity::Transform() const {
	return m_transform;
}


} // namespace inl::gxeng
