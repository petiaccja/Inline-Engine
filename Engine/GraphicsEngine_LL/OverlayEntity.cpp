#include "OverlayEntity.hpp"

namespace inl::gxeng {



OverlayEntity::OverlayEntity() {
	m_color = mathfu::Vector4f(1.f);
	m_position = mathfu::Vector2f(0, 0);
	m_scale = mathfu::Vector2f(1, 1);
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


void OverlayEntity::SetColor(mathfu::Vector4f color) {
	m_color = color;
}


mathfu::Vector4f OverlayEntity::GetColor() const {
	return std::get<mathfu::Vector4f>(m_color);
}


void OverlayEntity::SetTexture(Image * texture) {
	m_color = texture;
}


Image* OverlayEntity::GetTexture() const {
	return std::get<Image*>(m_color);
}


void OverlayEntity::SetPosition(mathfu::Vector<float, 2> pos) {
	m_position = pos;
}


void OverlayEntity::SetRotation(float rotation) {
	m_rotation = rotation;
}


void OverlayEntity::SetScale(mathfu::Vector<float, 2> scale) {
	m_scale = scale;
}


mathfu::Vector<float, 2> OverlayEntity::GetPosition() const {
	return m_position;
}


float OverlayEntity::GetRotation() const {
	return m_rotation;
}


mathfu::Vector<float, 2> OverlayEntity::GetScale() const {
	return m_scale;
}


mathfu::Matrix<float, 4, 4> OverlayEntity::GetTransform() const {
	using Mat4 = mathfu::Matrix4x4f;
	using Vec3 = mathfu::Vector3f;

	auto scale = Mat4::FromScaleVector(Vec3(m_scale, 1.f));
	auto rotate = mathfu::Quaternionf::FromAngleAxis(m_rotation, Vec3(0, 0, -1)).ToMatrix4();
	auto translate = Mat4::FromTranslationVector(Vec3(m_position, 0));

	return translate * rotate * scale;
}


} // namespace inl::gxeng
