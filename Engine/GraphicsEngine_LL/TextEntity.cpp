#include "TextEntity.hpp"
#include "Font.hpp"

#include <BaseLibrary/Exception/Exception.hpp>
#include <BaseLibrary/StringUtil.hpp>

namespace inl::gxeng {


TextEntity::TextEntity() {}

void TextEntity::SetFont(const Font* font) {
	m_font = font;
}

void TextEntity::SetColor(Vec4 color) {
	m_color = color;
}

void TextEntity::SetText(std::u32string text) {
	m_text = std::move(text);
}

void TextEntity::SetFontSize(float size) {
	if (size <= 0.0f) {
		throw InvalidArgumentException("Font size must be a positive real number.");
	}
	m_fontSize = size;
}

void TextEntity::SetSize(const Vec2& size) {
	m_size = size;
}

const Font* TextEntity::GetFont() const {
	return m_font;
}

Vec4 TextEntity::GetColor() const {
	return m_color;
}

const std::u32string& TextEntity::GetText() const {
	return m_text;
}

float TextEntity::GetFontSize() const {
	return m_fontSize;
}

void TextEntity::SetAdditionalClip(RectF clipRectangle, Mat33 transform) {
	m_clipRect = clipRectangle;
	m_clipRectTransform = transform;
}
std::pair<RectF, Mat33> TextEntity::GetAdditionalClip() const {
	return { m_clipRect, m_clipRectTransform };
}
void TextEntity::EnableAdditionalClip(bool enabled) {
	m_clipEnabled = enabled;
}
bool TextEntity::IsAdditionalClipEnabled() const {
	return m_clipEnabled;
}


void TextEntity::SetHorizontalAlignment(float alignment) {
	m_alignment.x = alignment;
}
void TextEntity::SetVerticalAlignment(float alignment) {
	m_alignment.y = alignment;
}
float TextEntity::GetHorizontalAlignment() const {
	return m_alignment.x;
}
float TextEntity::GetVerticalAlignment() const {
	return m_alignment.y;
}

float TextEntity::CalculateTextWidth() const {
	if (m_font) {
		// Convert string to UCS-4 code-points.
		std::u32string text = EncodeString<char32_t>(m_text);

		// Accumulate length.
		float width = 0.0f;
		for (auto ch : text) {
			if (!m_font->IsCharacterSupported(ch)) {
				continue;
			}
			width += m_fontSize * m_font->GetGlyphInfo(ch).advance;
		}
		return width;
	}
	else {
		throw InvalidCallException("Cannot calculate text metrics because no font is set.");
	}
}

float TextEntity::CalculateTextHeight() const {
	if (m_font) {
		return m_font->CalculateTextHeight(m_fontSize);
	}
	else {
		throw InvalidCallException("Cannot calculate text metrics because no font is set.");
	}
}

const Vec2& TextEntity::GetSize() const {
	return m_size;
}


void TextEntity::SetZDepth(float z) {
	m_zDepth = z;
}
float TextEntity::GetZDepth() const {
	return m_zDepth;
}


} // namespace inl::gxeng
