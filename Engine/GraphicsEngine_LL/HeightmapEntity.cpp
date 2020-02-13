#include "HeightmapEntity.hpp"

namespace inl::gxeng {


void HeightmapEntity::SetMesh(std::shared_ptr<Mesh> mesh) {
	m_mesh = mesh;
}

std::shared_ptr<IMesh> HeightmapEntity::GetMesh() const {
	return m_mesh;
}

const std::shared_ptr<Mesh>& HeightmapEntity::GetMeshNative() const {
	return m_mesh;
}

void HeightmapEntity::SetMaterial(std::shared_ptr<Material> material) {
	m_material = material;
}

std::shared_ptr<IMaterial> HeightmapEntity::GetMaterial() const {
	return m_material;
}

const std::shared_ptr<Material>& HeightmapEntity::GetMaterialNative() const {
	return m_material;
}

void HeightmapEntity::SetHeightmap(std::shared_ptr<Image> heightmap) {
	m_heightmap = heightmap;
}

std::shared_ptr<IImage> HeightmapEntity::GetHeightmap() const {
	return m_heightmap;
}

const std::shared_ptr<Image>& HeightmapEntity::GetHeightmapNative() const {
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

void HeightmapEntity::SetUvSize(Vec2 size) {
	m_uvSize = size;
}

Vec2 HeightmapEntity::GetUvSize() const {
	return m_uvSize;
}


} // namespace inl::gxeng