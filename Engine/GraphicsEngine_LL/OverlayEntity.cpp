#include "OverlayEntity.hpp"

namespace inl::gxeng {



OverlayEntity::OverlayEntity() {
	m_color = Vec4(1.f);
	m_mesh = nullptr;
}


void OverlayEntity::SetMesh(Mesh * mesh) {
	m_mesh = mesh;
}


Mesh * OverlayEntity::GetMesh() const {
	return m_mesh;
}


OverlayEntity::eSurfaceType OverlayEntity::GetSurfaceType() const {
	if (std::holds_alternative<Image*>(m_color)) {
		return TEXTURED;
	}
	return COLORED;
}


void OverlayEntity::SetColor(Vec4 color) {
	m_color = color;
}


Vec4 OverlayEntity::GetColor() const {
	return std::get<Vec4>(m_color);
}


void OverlayEntity::SetTexture(Image * texture) {
	m_color = texture;
}


Image* OverlayEntity::GetTexture() const {
	return std::get<Image*>(m_color);
}


} // namespace inl::gxeng
