#pragma once

#include <InlineMath.hpp>

#include <variant>

namespace inl::gxeng {


class Mesh;
class Image;


class OverlayEntity {
public:
	enum SurfaceType { TEXTURED, COLORED };

public:
	OverlayEntity();

	void SetVisible(bool visible);
	bool GetVisible() const;

	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const;

	SurfaceType GetSurfaceType() const;

	void SetColor(Vec4 color);
	Vec4 GetColor() const;
	void SetTexture(Image* texture);
	Image* GetTexture() const;

	void SetPosition(Vec2 pos);
	void SetRotation(float rotation);
	void SetScale(Vec2 scale);

	Vec2 GetPosition() const;
	Vec2 GetScale() const;
	float GetRotation() const;

	Mat44 GetTransform() const;

private:
	Mesh* m_mesh;

	bool m_visible;
	std::variant<Image*, Vec4> m_color;

	Vec2 m_position;
	Vec2 m_scale;
	float m_rotation;
};


} // namespace inl::gxeng
