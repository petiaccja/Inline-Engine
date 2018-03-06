#pragma once

#include "TextEntity.hpp"
#include <BaseLibrary/Exception/Exception.hpp>

namespace inl::gxeng {


TextEntity::TextEntity() {}

void TextEntity::SetFont(const Font* font) {
	m_font = font;
}

void TextEntity::SetColor(Vec4 color) {
	m_color = color;
}

void TextEntity::SetText(std::string text) {
	m_text = std::move(text);
}

void TextEntity::SetFontSize(float size) {
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

const std::string& TextEntity::GetText() const {
	return m_text;
}

float TextEntity::GetFontSize() const {
	return m_fontSize;
}

float TextEntity::GetTextWidth() const {
	throw NotImplementedException();
}

float TextEntity::GetTextHeight() const {
	throw NotImplementedException();
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
