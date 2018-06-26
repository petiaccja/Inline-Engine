#include "OverlayEntity.hpp"

namespace inl::gxeng {



OverlayEntity::OverlayEntity() {
	m_color = Vec4(1.f);
}


void OverlayEntity::SetMesh(Mesh * mesh) {
	m_mesh = mesh;
}


Mesh * OverlayEntity::GetMesh() const {
	return m_mesh;
}

void OverlayEntity::SetColor(Vec4 color) {
	m_color = color;
}


Vec4 OverlayEntity::GetColor() const {
	return Vec4(m_color);
}


void OverlayEntity::SetTexture(Image * texture) {
	m_texture = texture;
}


Image* OverlayEntity::GetTexture() const {
	return m_texture;
}


void OverlayEntity::SetZDepth(float z) {
	m_zDepth = z;
}
float OverlayEntity::GetZDepth() const {
	return m_zDepth;
}



} // namespace inl::gxeng
