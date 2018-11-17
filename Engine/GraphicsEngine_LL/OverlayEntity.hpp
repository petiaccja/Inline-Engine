#pragma once

#include "Image.hpp"
#include "Mesh.hpp"

#include <BaseLibrary/Transformable.hpp>
#include <BaseLibrary/Rect.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>

#include <InlineMath.hpp>

namespace inl::gxeng {


class OverlayEntity : public IOverlayEntity, public Transformable2DN {

public:
	OverlayEntity();

	/// <summary> Sets a 2D triangle mesh as shape of the object. </summary>
	/// <remarks> Set to null to draw a unit rectangle with corners (0,0) and (1,1).
	///		<para/> Mesh must have at least Vec2 position and Vec2 texcoord0. </remarks>
	void SetMesh(IMesh* mesh) { SetMesh(static_cast<Mesh*>(mesh)); }
	void SetMesh(Mesh* mesh);
	Mesh* GetMesh() const override;

	/// <summary> Solid color of the object. Multiplied together with texture color. </summary>
	void SetColor(Vec4 color) override;
	Vec4 GetColor() const override;

	/// <summary> Texture color of the object. Multiplied together with base color. </summary>
	/// <remarks> Set to null if you only want solid color. </remarks>
	void SetTexture(IImage* texture) { SetTexture(static_cast<Image*>(texture)); }
	void SetTexture(Image* texture);
	Image* GetTexture() const override;

	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z) override;
	float GetZDepth() const override;


	/// <summary> The text is optionally clipped against an additional bounding box as well. </summary>
	/// <param name="clipRectangle"> The rectangle to clip against. </param>
	/// <param name="transform"> How to transform the <paramref name="clipRectangle"/> before clipping. </param>
	void SetAdditionalClip(RectF clipRectangle, Mat33 transform) override;
	/// <summary> Returns the additional cliiping rectangle and its transform. </summary>
	std::pair<RectF, Mat33> GetAdditionalClip() const override;
	/// <summary> Enabled or disables the usage of the additional clip rectangle. </summary>
	void EnableAdditionalClip(bool enabled) override;
	/// <summary> Check if additional clip rectangle is active. </summary>
	bool IsAdditionalClipEnabled() const override;

private:
	Mesh* m_mesh = nullptr;
	Image* m_texture = nullptr;
	Vec4 m_color;
	float m_zDepth = 0.0f;

	RectF m_clipRect;
	Mat33 m_clipRectTransform;
	bool m_clipEnabled = false;
};


} // namespace inl::gxeng
