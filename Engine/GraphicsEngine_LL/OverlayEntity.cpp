#include "OverlayEntity.hpp"

namespace inl::gxeng {



OverlayEntity::OverlayEntity() {
	m_color = Vec4(1.f);
	m_position = Vec2(0, 0);
	m_scale = Vec2(1, 1);
	m_rotation = 0;
	m_visible = true;
	m_mesh = nullptr;
}


void OverlayEntity::SetVisible(bool visible) {
	m_visible = visible;
}


bool OverlayEntity::GetVisible() const {
	return m_visible;
}


void OverlayEntity::SetMesh(Mesh * mesh) {
	m_mesh = mesh;
}


Mesh * OverlayEntity::GetMesh() const {
	return m_mesh;
}


OverlayEntity::SurfaceType OverlayEntity::GetSurfaceType() const {
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


void OverlayEntity::SetPosition(Vec2 pos) {
	m_position = pos;
}


void OverlayEntity::SetRotation(float rotation) {
	m_rotation = rotation;
}


void OverlayEntity::SetScale(Vec2 scale) {
	m_scale = scale;
}


Vec2 OverlayEntity::GetPosition() const {
	return m_position;
}


float OverlayEntity::GetRotation() const {
	return m_rotation;
}


Vec2 OverlayEntity::GetScale() const {
	return m_scale;
}


Mat44 OverlayEntity::GetTransform() const {
	auto scale = Mat44::Scale(Vec3(m_scale, 1.f));
	auto rotate = Mat44::RotationZ(-m_rotation);
	auto translate = Mat44::Translation(Vec3(m_position, 0));

	return scale * rotate * translate;
}


} // namespace inl::gxeng
