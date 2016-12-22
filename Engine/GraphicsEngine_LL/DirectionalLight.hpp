#pragma once

#include <mathfu/mathfu_exc.hpp>

namespace inl::gxeng {

class DirectionalLight {
public:
	DirectionalLight() = default;
	DirectionalLight(mathfu::Vector3f direction, mathfu::Vector3f color);

	void SetDirection(const mathfu::Vector3f& dir);
	void SetColor(const mathfu::Vector3f& color);

	mathfu::Vector3f GetDirection() const;
	mathfu::Vector3f GetColor() const;

protected:
	mathfu::Vector3f m_direction;
	mathfu::Vector3f m_color;
};

} // namespace inl::gxeng