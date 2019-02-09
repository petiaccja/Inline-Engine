#pragma once

#include "Control.hpp"
#include "GraphicalControl.hpp"
#include "GraphicsContext.hpp"

#include <GraphicsEngine/Scene/IOverlayEntity.hpp>


namespace inl::gui {

// THINK ABOUT:
// Why not make gui::Sprite and gui::Text a Control themselves?
// Then they could properly add themselves to real scenes.
// Normal controls could just simply add them as sub-controls.


class Sprite final : public Control, public GraphicalControl {
public:
	/// <summary> Creates an invisible placeholder sprite. </summary>
	Sprite();

	/// <summary> Creates an invisible placeholder sprite with the properties of another Sprite. </summary>
	Sprite(const Sprite& other);

	/// <summary> Moves the sprite. The original will become invalid and cannot be further used. </summary>
	Sprite(Sprite&& other) noexcept;

	void SetContext(const GraphicsContext& context) override;
	void ClearContext() override;


	//-------------------------------------
	// Control
	//-------------------------------------

	// Sizing
	void SetSize(const Vec2& size) override;
	Vec2 GetSize() const override;
	Vec2 GetPreferredSize() const override;
	Vec2 GetMinimumSize() const override;

	// Position
	void SetPosition(const Vec2& position) override;
	Vec2 GetPosition() const override;
	float SetDepth(float depth) override;
	float GetDepth() const override;

	// Rotation
	void SetRotation(float angle);
	float GetRotation() const;

	bool HitTest(const Vec2& point) const override;

	//-------------------------------------
	// OverlayEntity
	//-------------------------------------
	void SetMesh(gxeng::IMesh* mesh);
	gxeng::IMesh* GetMesh() const;

	void SetColor(Vec4 color);
	Vec4 GetColor() const;

	void SetTexture(gxeng::IImage* texture);
	gxeng::IImage* GetTexture() const;

	void SetAdditionalClip(RectF clipRectangle, Mat33 transform);
	std::pair<RectF, Mat33> GetAdditionalClip() const;
	void EnableAdditionalClip(bool enabled);
	bool IsAdditionalClipEnabled() const;

private:
	static void CopyProperties(const gxeng::IOverlayEntity& source, gxeng::IOverlayEntity& target);

private:
	std::unique_ptr<gxeng::IOverlayEntity> m_entity;
	GraphicsContext m_context;
};


} // namespace inl::gui