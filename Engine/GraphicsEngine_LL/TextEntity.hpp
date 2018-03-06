#pragma once

#include <BaseLibrary/Transformable.hpp>
#include <BaseLibrary/Rect.hpp>

#include <InlineMath.hpp>
#include <variant>

namespace inl::gxeng {


class Font;


class TextEntity : public Transformable2DN {
public:
	TextEntity();

	void SetFont(const Font* font);
	void SetColor(Vec4 color);
	void SetText(std::string text);
	void SetFontSize(float size);
	void SetSize(const Vec2& size);

	const Font* GetFont() const;
	Vec4 GetColor() const;
	const std::string& GetText() const;
	float GetFontSize() const;
	const Vec2& GetSize() const;

	float GetTextWidth() const;
	float GetTextHeight() const;

	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z);
	float GetZDepth() const;
private:
	Vec4 m_color = {1,0,0,1};
	float m_fontSize = 16;
	std::string m_text = u8"PLACEHOLDER";
	const Font* m_font = nullptr;
	float m_zDepth = 0.0f;
	Vec2 m_size = {16, 16};
};


} // namespace inl::gxeng
