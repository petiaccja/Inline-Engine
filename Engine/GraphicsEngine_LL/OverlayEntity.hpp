#pragma once

#include "BaseLibrary/Transformable.hpp"

#include <InlineMath.hpp>
#include <variant>

namespace inl::gxeng {


class Mesh;
class Image;


class OverlayEntity : public Transformable2DN {
public:
	enum eSurfaceType { TEXTURED, COLORED };

public:
	OverlayEntity();

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const;

	eSurfaceType GetSurfaceType() const;

	void SetColor(Vec4 color);
	Vec4 GetColor() const;
	void SetTexture(Image* texture);
	Image* GetTexture() const;

private:
	Mesh* m_mesh;
	std::variant<Image*, Vec4> m_color;
};


} // namespace inl::gxeng
