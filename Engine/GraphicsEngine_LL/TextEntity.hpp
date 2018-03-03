#pragma once

#include "BaseLibrary/Transformable.hpp"

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

	const Font* GetFont() const;
	Vec4 GetColor() const;
	const std::string& GetText() const;
	float GetFontSize() const;

	float GetTextWidth() const;
	float GetTextHeight() const;

	/// <summary> Z-Depth determines which 2D entity lays over the other. </summary>
	/// <remarks> Number are not limited to [0,1], anything is fine. Don't pass NaN and Inf. </remarks>
	void SetZDepth(float z);
	float GetZDepth() const;
private:
	Vec4 m_color;
	std::string m_text;
	const Font* m_font;
	float m_zDepth = 0.0f;
};


} // namespace inl::gxeng
