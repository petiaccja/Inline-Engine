#pragma once

#include <mathfu/vector.h>
#include <mathfu/vector_3.h>
#include <mathfu/quaternion.h>
#include <mathfu/matrix_4x4.h>

#include <mathfu/mathfu_exc.hpp>

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

	void SetColor(mathfu::Vector4f color);
	mathfu::Vector4f GetColor() const;
	void SetTexture(Image* texture);
	Image* GetTexture() const;

	void SetPosition(mathfu::Vector<float, 2> pos);
	void SetRotation(float rotation);
	void SetScale(mathfu::Vector<float, 2> scale);

	mathfu::Vector<float, 2> GetPosition() const;
	mathfu::Vector<float, 2> GetScale() const;
	float GetRotation() const;

	mathfu::Matrix<float, 4, 4> GetTransform() const;

private:
	Mesh* m_mesh;

	bool m_visible;
	std::variant<Image*, mathfu::Vector4f> m_color;

	mathfu::Vector<float, 2> m_position;
	mathfu::Vector<float, 2> m_scale;
	float m_rotation;
};


} // namespace inl::gxeng
