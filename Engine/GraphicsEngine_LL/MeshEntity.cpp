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


void MeshEntity::SetPosition(Vec3 pos) {
	m_position = pos;
}


void MeshEntity::SetRotation(mathfu::Quaternion<float> rotation) {
	m_rotation = rotation;
}


void MeshEntity::SetScale(Vec3 scale) {
	m_scale = scale;
}


Vec3 MeshEntity::GetPosition() const {
	return m_position;
}


mathfu::Quaternion<float> MeshEntity::GetRotation() const {
	return m_rotation;
}


Vec3 MeshEntity::GetScale() const {
	return m_scale;
}


Mat44 MeshEntity::GetTransform() const {
	mathfu::Vector<float, 3> axis;
	float angle;
	m_rotation.ToAngleAxis(&angle, &axis);
	return  Mat44::Scale(m_scale) * Mat44::RotationAxisAngle(Vec3{ axis.x(), axis.y(), axis.z() }, angle) * Mat44::Translation(m_position);
}


}
}
