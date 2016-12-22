#include "DirectionalLight.hpp"

namespace inl::gxeng {


DirectionalLight::DirectionalLight(mathfu::Vector3f direction, mathfu::Vector3f color
):
	m_direction(direction),
	m_color(color)
{}


void DirectionalLight::SetDirection(const mathfu::Vector3f& dir) {
	m_direction = dir;
}


void DirectionalLight::SetColor(const mathfu::Vector3f& color) {
	m_color = color;
}


mathfu::Vector3f DirectionalLight::GetDirection() const {
	return m_direction;
}


mathfu::Vector3f DirectionalLight::GetColor() const {
	return m_color;
}


} // namespace inl::gxeng
