#pragma once

#include <InlineMath.hpp>


namespace inl::gxeng
{
	

class IDirectionalLight {
public:
	virtual ~IDirectionalLight() = default;

	virtual void SetDirection(const Vec3& dir) = 0;
	virtual void SetColor(const Vec3& color) = 0;

	virtual Vec3 GetDirection() const = 0;
	virtual Vec3 GetColor() const = 0;
};


}