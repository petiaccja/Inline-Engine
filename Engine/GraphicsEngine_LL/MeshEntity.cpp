#include "MeshEntity.hpp"

namespace inl {
namespace gxeng {


MeshEntity::MeshEntity() :
	m_mesh(nullptr),
	m_material(nullptr),
	m_position(0, 0, 0),
	m_rotation(1, 0, 0, 0),
	m_scale(1, 1, 1)
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


void MeshEntity::SetPosition(const Vec3& pos) {
	m_position = pos;
}


void MeshEntity::SetRotation(const Quat& rotation) {
	m_rotation = rotation;
}


void MeshEntity::SetScale(const Vec3& scale) {
	m_scale = scale;
}


Vec3 MeshEntity::GetPosition() const {
	return m_position;
}


Quat MeshEntity::GetRotation() const {
	return m_rotation;
}


Vec3 MeshEntity::GetScale() const {
	return m_scale;
}


Mat44 MeshEntity::GetTransform() const {
	return  Mat44::Scale(m_scale) * Mat44(m_rotation) * Mat44::Translation(m_position);
}


}
}
