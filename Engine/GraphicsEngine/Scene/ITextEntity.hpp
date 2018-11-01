#pragma once


#include <InlineMath.hpp>
#include <BaseLibrary/Transformable.hpp>
#include <BaseLibrary/Rect.hpp>

#include <utility>


namespace inl::gxeng {


class IFont;


class ITextEntity : public virtual ITransformable2DN {
public:
	virtual ~ITextEntity() = default;

	/// <summary> Text is drawn using given font face. </summary>
	virtual void SetFont(const IFont* font) = 0;
	/// <summary> Returns the currently used font face. </summary>
	virtual const IFont* GetFont() const = 0;


	/// <summary> Sets the height of characters, in 2D camera units. </summary>
	virtual void SetFontSize(float size) = 0;
	/// <summary> Returns the current height of characters. </summary>
	virtual float GetFontSize() const = 0;


	/// <summary> This text is drawn to the screen. </summary>
	virtual void SetText(std::u32string text) = 0;
	/// <summary> Returns the currently drawn text. </summary>
	virtual const std::u32string& GetText() const = 0;


	/// <summary> Sets a solid color for the whole text. </summary>
	virtual void SetColor(Vec4 color) = 0;
	/// <summary> Returns current solid text color. </summary>
	virtual Vec4 GetColor() const = 0;


	/// <summary> The text goes into a box of this size. 
	///		The center of the box is the position of the entity. </summary>
	/// <remarks> Size is in the same units as the font height, and gets
	///		physical meaning through the 2D camera that view the overlays.
	///		<para/> Also clips the text. </remarks>
	virtual void SetSize(const Vec2& size) = 0;
	/// <summary> Return current bounding box size. </summary>
	virtual const Vec2& GetSize() const = 0;


	/// <summary> The text is optionally clipped against an additional bounding box as well. </summary>
	/// <param name="clipRectangle"> The rectangle to clip against. </param>
	/// <param name="transform"> How to transform the <paramref name="clipRectangle"/> before clipping. </param>
	virtual void SetAdditionalClip(RectF clipRectangle, Mat33 transform) = 0;
	/// <summary> Returns the additional cliiping rectangle and its transform. </summary>
	virtual std::pair<RectF, Mat33> GetAdditionalClip() const = 0;
	/// <summary> Enabled or disables the usage of the additional clip rectangle. </summary>
	virtual void EnableAdditionalClip(bool enabled) = 0;
	/// <summary> Check if additional clip rectangle is active. </summary>
	virtual bool IsAdditionalClipEnabled() const = 0;


	/// <summary> -1 aligns left, 0 aligns center, 1 aligns right. Any value in [-1,1] is acceptable. </summary>
	/// <remarks> Use helper static constants defined in TextEntity. </remarks>
	virtual void SetHorizontalAlignment(float alignment) = 0;
	/// <summary> -1 aligns bottom, 0 aligns center, 1 aligns top. Any value in [-1,1] is acceptable. </summary>
	/// <remarks> Use helper static constants defined in TextEntity. </remarks>
	virtual void SetVerticalAlignment(float alignment) = 0;
	/// <summary> See <see cref="SetHorizontalAlignment"/> </summary>
	virtual float GetHorizontalAlignment() const = 0;
	/// <summary> See <see cref="SetVerticalAlignment"/> </summary>
	virtual float GetVerticalAlignment() const = 0;


	/// <summary> Return how many camera units the text takes when rendered. </summary>
	virtual float CalculateTextWidth() const = 0;
	/// <summary> Return the height of a line of text without top and bottom line spacing. </summary>
	virtual float CalculateTextHeight() const = 0;

	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	virtual void SetZDepth(float z) = 0;
	virtual float GetZDepth() const = 0;

	/// <summary> Align to the left when specified to <see cref="SetHorizontalAlignment"/>. </summary>
	static constexpr float ALIGN_LEFT = -1.0f;
	/// <summary> Align to the right when specified to <see cref="SetHorizontalAlignment"/>. </summary>
	static constexpr float ALIGN_RIGHT = 1.0f;
	/// <summary> Align to the bottom when specified to <see cref="SetVerticalAlignment"/>. </summary>
	static constexpr float ALIGN_BOTTOM = -1.0f;
	/// <summary> Align to the top when specified to <see cref="SetVerticalAlignment"/>. </summary>
	static constexpr float ALIGN_TOP = 1.0f;
	/// <summary> Align to the top when specified to either <see cref="SetHorizontalAlignment"/> or <see cref="SetVerticalAlignment"/>. </summary>
	static constexpr float ALIGN_CENTER = 0.0f;
};


} // namespace inl::gxeng
