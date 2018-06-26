#pragma once

#include <InlineMath.hpp>

namespace inl::tool {


class Drawable {
public:
	virtual ~Drawable() = default;

	virtual const Drawable* Intersect(Vec2 point) const = 0;

	virtual void SetPosition(Vec2 position) = 0;
	virtual Vec2 GetPosition() const = 0;

	virtual void SetSize(Vec2 size) = 0;
	virtual Vec2 GetSize() const = 0;

	virtual void SetDepth(float depth) = 0;
	virtual float GetDepth() const = 0;
};


}