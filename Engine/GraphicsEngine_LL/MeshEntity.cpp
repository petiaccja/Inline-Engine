#include "MeshEntity.hpp"

namespace inl {
namespace gxeng {


MeshEntity::MeshEntity() :
	m_mesh(nullptr),
	m_material(nullptr),
	m_position(0, 0, 0),
	m_rotation(1, 0, 0, 0),
	m_scale(1, 1, 1),
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

void MeshEntity::InitPosition(Vec3 pos) {
	m_prevPosition = pos;
	m_position = pos;
}


void MeshEntity::InitRotation(Quat rotation) {
	m_prevRotation = rotation;
	m_rotation = rotation;
}


void MeshEntity::InitScale(Vec3 scale) {
	m_prevScale = scale;
	m_scale = scale;
}



void MeshEntity::SetPosition(const Vec3& pos) {
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

Vec3 MeshEntity::GetPrevPosition() const {
	return m_prevPosition;
}


Quat MeshEntity::GetPrevRotation() const {
	return m_prevRotation;
}


Vec3 MeshEntity::GetPrevScale() const {
	return m_prevScale;
}

Mat44 MeshEntity::GetTransform() const {
	return Mat44::Scale(m_scale) * Mat44(m_rotation) * Mat44::Translation(m_position);
}

Mat44 MeshEntity::GetPrevTransform() const {
	return Mat44::Scale(m_prevScale) * Mat44(m_prevRotation) * Mat44::Translation(m_prevPosition);
}

}
}
