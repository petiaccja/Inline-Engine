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


void OverlayEntity::SetAdditionalClip(RectF clipRectangle, Mat33 transform) {
	m_clipRect = clipRectangle;
	m_clipRectTransform = transform;
}
std::pair<RectF, Mat33> OverlayEntity::GetAdditionalClip() const {
	return { m_clipRect, m_clipRectTransform };
}
void OverlayEntity::EnableAdditionalClip(bool enabled) {
	m_clipEnabled = enabled;
}
bool OverlayEntity::IsAdditionalClipEnabled() const {
	return m_clipEnabled;
}




} // namespace inl::gxeng
