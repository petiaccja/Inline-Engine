#pragma once

#include <GraphicsEngine/Scene/IDirectionalLight.hpp>

#include <InlineMath.hpp>


namespace inl::gxeng {

class DirectionalLight : public IDirectionalLight {
public:
	DirectionalLight() = default;
	DirectionalLight(Vec3 direction, Vec3 color);

	void SetDirection(const Vec3& dir) override;
	void SetColor(const Vec3& color) override;

	Vec3 GetDirection() const override;
	Vec3 GetColor() const override;

protected:
	Vec3 m_direction = { 0, 0, -1 };
	Vec3 m_color = { 1, 0, 0 };
};

} // namespace inl::gxeng