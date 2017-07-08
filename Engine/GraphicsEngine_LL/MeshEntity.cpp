#include "MeshEntity.hpp"

namespace inl {
namespace gxeng {


MeshEntity::MeshEntity() :
	m_mesh(nullptr),
	m_material(nullptr),
	m_position(0, 0, 0),
	m_rotation(1, 0, 0, 0),
	m_scale(1, 1, 1)
	m_prevPosition(m_position),
	m_prevRotation(m_rotation),
	m_prevScale(m_scale)
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

void MeshEntity::InitPosition(mathfu::Vector<float, 3> pos) {
	m_prevPosition = pos;
	m_position = pos;
}


void MeshEntity::InitRotation(mathfu::Quaternion<float> rotation) {
	m_prevRotation = rotation;
	m_rotation = rotation;
}


void MeshEntity::InitScale(mathfu::Vector<float, 3> scale) {
	m_prevScale = scale;
	m_scale = scale;
}



void MeshEntity::SetPosition(mathfu::Vector<float, 3> pos) {
	m_prevPosition = m_position;
	m_position = pos;
}


void MeshEntity::SetRotation(const Quat& rotation) {
	m_prevRotation = m_rotation;
	m_rotation = rotation;
}


void MeshEntity::SetScale(const Vec3& scale) {
	m_prevScale = m_scale;
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

mathfu::Vector<float, 3> MeshEntity::GetPrevPosition() const {
	return m_prevPosition;
}


mathfu::Quaternion<float> MeshEntity::GetPrevRotation() const {
	return m_prevRotation;
}


mathfu::Vector<float, 3> MeshEntity::GetPrevScale() const {
	return m_prevScale;
}


mathfu::Matrix<float, 4, 4> MeshEntity::GetTransform() const {
	return  Mat44::Scale(m_scale) * Mat44(m_rotation) * Mat44::Translation(m_position);
}

mathfu::Matrix<float, 4, 4> MeshEntity::GetPrevTransform() const {
	using Mat4 = mathfu::Matrix<float, 4, 4>;

	return Mat4::FromTranslationVector(m_prevPosition) * m_prevRotation.ToMatrix4() * Mat4::FromScaleVector(m_prevScale);
}

}
}
