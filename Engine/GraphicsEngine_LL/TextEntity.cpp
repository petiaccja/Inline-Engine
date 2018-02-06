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
private:
	Vec4 m_color;
	std::string m_text;
	const Font* m_font;
};


} // namespace inl::gxeng
