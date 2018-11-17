#pragma once

#include <GraphicsEngine/Scene/ITextEntity.hpp>

#include <BaseLibrary/Transformable.hpp>
#include <BaseLibrary/Rect.hpp>

#include <InlineMath.hpp>
#include <variant>
#include <utility>

#include "Font.hpp"


namespace inl::gxeng {


class TextEntity : public ITextEntity, public Transformable2DN {
public:
	TextEntity();

	/// <summary> Text is drawn using given font face. </summary>
	void SetFont(const IFont* font) override { SetFont(static_cast<const Font*>(font)); }
	void SetFont(const Font* font);
	/// <summary> Returns the currently used font face. </summary>
	const Font* GetFont() const override;


	/// <summary> Sets the height of characters, in 2D camera units. </summary>
	void SetFontSize(float size) override;
	/// <summary> Returns the current height of characters. </summary>
	float GetFontSize() const override;


	/// <summary> This text is drawn to the screen. </summary>
	void SetText(std::u32string text) override;
	/// <summary> Returns the currently drawn text. </summary>
	const std::u32string& GetText() const override;


	/// <summary> Sets a solid color for the whole text. </summary>
	void SetColor(Vec4 color) override;
	/// <summary> Returns current solid text color. </summary>
	Vec4 GetColor() const override;


	/// <summary> The text goes into a box of this size. 
	///		The center of the box is the position of the entity. </summary>
	/// <remarks> Size is in the same units as the font height, and gets
	///		physical meaning through the 2D camera that view the overlays.
	///		<para/> Also clips the text. </remarks>
	void SetSize(const Vec2& size) override;
	/// <summary> Return current bounding box size. </summary>
	const Vec2& GetSize() const override;


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


	/// <summary> -1 aligns left, 0 aligns center, 1 aligns right. Any value in [-1,1] is acceptable. </summary>
	/// <remarks> Use helper static constants defined in TextEntity. </remarks>
	void SetHorizontalAlignment(float alignment) override;
	/// <summary> -1 aligns bottom, 0 aligns center, 1 aligns top. Any value in [-1,1] is acceptable. </summary>
	/// <remarks> Use helper static constants defined in TextEntity. </remarks>
	void SetVerticalAlignment(float alignment) override;
	/// <summary> See <see cref="SetHorizontalAlignment"/> </summary>
	float GetHorizontalAlignment() const override;
	/// <summary> See <see cref="SetVerticalAlignment"/> </summary>
	float GetVerticalAlignment() const override;


	/// <summary> Return how many camera units the text takes when rendered. </summary>
	float CalculateTextWidth() const override;
	/// <summary> Return the height of a line of text without top and bottom line spacing. </summary>
	float CalculateTextHeight() const override;

	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z) override;
	float GetZDepth() const override;

private:
	Vec4 m_color = { 1,0,0,1 };
	float m_fontSize = 16;
	std::u32string m_text;
	const Font* m_font = nullptr;
	float m_zDepth = 0.0f;
	Vec2 m_size = { 16, 16 };

	RectF m_clipRect;
	Mat33 m_clipRectTransform;
	bool m_clipEnabled = false;
	Vec2 m_alignment = { ALIGN_CENTER, ALIGN_CENTER };
};


} // namespace inl::gxeng
