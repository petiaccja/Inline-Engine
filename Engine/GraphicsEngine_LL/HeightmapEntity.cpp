#include "HeightmapEntity.hpp"

namespace inl::gxeng {


void HeightmapEntity::SetMesh(Mesh* mesh) {
	m_mesh = mesh;
}

Mesh* HeightmapEntity::GetMesh() const {
	return m_mesh;
}

void HeightmapEntity::SetMaterial(Material* material) {
	m_material = material;
}

Material* HeightmapEntity::GetMaterial() const {
	return m_material;
}

void HeightmapEntity::SetHeightmap(Image* heightmap) {
	m_heightmap = heightmap;
}

Image* HeightmapEntity::GetHeightmap() const {
	return m_heightmap;
}

void HeightmapEntity::SetDirection(Vec3 direction) {
	m_direction = direction.SafeNormalized({ 0, 0, 1 });
}

Vec3 HeightmapEntity::GetDirection() const {
	return m_direction;
}

void HeightmapEntity::SetMagnitude(float magnitude) {
	m_magnitude = magnitude;
}

float HeightmapEntity::GetMagnitude() const {
	return m_magnitude;
}

void HeightmapEntity::SetOffset(float offset) {
	m_offset = offset;
}

float HeightmapEntity::GetOffset() const {
	return m_offset;
}


} // namespace inl::gxeng