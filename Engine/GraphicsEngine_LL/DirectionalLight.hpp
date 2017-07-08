#pragma once

#include <InlineMath.hpp>

namespace inl::gxeng {

class DirectionalLight {
public:
	DirectionalLight() = default;
	DirectionalLight(Vec3 direction, Vec3 color);

	void SetDirection(const Vec3& dir);
	void SetColor(const Vec3& color);

	Vec3 GetDirection() const;
	Vec3 GetColor() const;

protected:
	Vec3 m_direction;
	Vec3 m_color;
};

} // namespace inl::gxeng