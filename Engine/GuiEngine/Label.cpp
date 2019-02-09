#include "Label.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"



namespace inl::gui {


Label::Label() {
	AddChild(m_text);
	m_text.SetDepth(0.1f);
	m_text.SetHorizontalAlignment(-1);
}

void Label::SetSize(const Vec2& size) {
	m_text.SetSize(size);
}

Vec2 Label::GetSize() const {
	return m_text.GetSize();
}

Vec2 Label::GetPreferredSize() const {
	if (m_text.GetFont()) {
		return { std::ceil(m_text.CalculateTextWidth()), std::ceil(m_text.CalculateTextHeight()) };
	}
	else {
		return { 10, 10 };
	}
}


Vec2 Label::GetMinimumSize() const {
	return { 0.0f, 0.0f };
}


void Label::SetPosition(const Vec2& position) {
	m_text.SetPosition(position);
}


Vec2 Label::GetPosition() const {
	return m_text.GetPosition();
}


void Label::SetHorizontalAlignment(float alignment) {
	m_text.SetHorizontalAlignment(alignment);
}


void Label::SetVerticalAlignment(float alignment) {
	m_text.SetVerticalAlignment(alignment);
}



void Label::SetText(std::u32string text) {
	m_text.SetText(std::move(text));
}
const std::u32string& Label::GetText() const {
	return m_text.GetText();
}

float Label::SetDepth(float depth) {
	m_text.SetDepth(depth);
	return 1.0f;
}

float Label::GetDepth() const {
	return m_text.GetDepth();
}


} // namespace inl::gui