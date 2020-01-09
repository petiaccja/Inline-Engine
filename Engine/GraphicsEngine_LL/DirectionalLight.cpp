#include "DirectionalLight.hpp"

namespace inl::gxeng {


DirectionalLight::DirectionalLight(Vec3 direction, Vec3 color) : m_direction(direction.Normalized()),
																 m_color(color) {}


void DirectionalLight::SetDirection(const Vec3& dir) {
	m_direction = dir.Normalized();
}


void DirectionalLight::SetColor(const Vec3& color) {
	m_color = color;
}


Vec3 DirectionalLight::GetDirection() const {
	return m_direction;
}


Vec3 DirectionalLight::GetColor() const {
	return m_color;
}


} // namespace inl::gxeng
