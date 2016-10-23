#include "MeshEntity.hpp"

namespace inl {
namespace gxeng {


MeshEntity::MeshEntity() :
	m_mesh(nullptr),
	m_position(0, 0, 0),
	m_rotation(0, mathfu::Vector<float, 3>(1, 0, 0)),
	m_scale(1, 1, 1)
{}



void MeshEntity::SetMesh(Mesh* mesh) {
	m_mesh = mesh;
}
Mesh* MeshEntity::GetMesh() const {
	return m_mesh;
}


void MeshEntity::SetPosition(mathfu::Vector<float, 3> pos) {
	m_position = pos;
}


void MeshEntity::SetRotation(mathfu::Quaternion<float> rotation) {
	m_rotation = rotation;
}


void MeshEntity::SetScale(mathfu::Vector<float, 3> scale) {
	m_scale = scale;
}


mathfu::Vector<float, 3> MeshEntity::GetPosition() const {
	return m_position;
}


mathfu::Quaternion<float> MeshEntity::GetRotation() const {
	return m_rotation;
}


mathfu::Vector<float, 3> MeshEntity::GetScale() const {
	return m_scale;
}


mathfu::Matrix<float, 4, 4> MeshEntity::GetTransform() const {
	using Mat4 = mathfu::Matrix<float, 4, 4>;

	return Mat4::FromTranslationVector(m_position) * m_rotation.ToMatrix4() * Mat4::FromScaleVector(m_scale);
}


}
}
