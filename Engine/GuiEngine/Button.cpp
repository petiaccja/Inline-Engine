#include "Button.hpp"

#include "Layout.hpp"

#include <BaseLibrary/Platform/System.hpp>


namespace inl::gui {


Button::Button() {
	OnEnterArea += [](auto...) {
		System::SetCursorVisual(eCursorVisual::ARROW, nullptr);
	};
	AddChild(m_background);
	AddChild(m_text);
}


void Button::SetSize(const Vec2& size) {
	m_background.SetSize(size);
	m_text.SetSize(size);
}


Vec2 Button::GetSize() const {
	return m_background.GetSize();
}


Vec2 Button::GetPreferredSize() const {
	if (m_text.GetFont()) {
		return { m_text.CalculateTextWidth(), m_text.CalculateTextHeight() };
	}
	else {
		return { 10, 10 };
	}
}


Vec2 Button::GetMinimumSize() const {
	return { 0.0f, 0.0f };
}


void Button::SetPosition(const Vec2& position) {
	m_background.SetPosition(position);
	m_text.SetPosition(position);
}


Vec2 Button::GetPosition() const {
	return m_background.GetPosition();
}


void Button::Update(float elapsed) {
	ColorF background;
	const auto& style = GetStyle();

	switch (m_stateTracker.Get()) {
		case eControlState::NORMAL: background = style.foreground; break;
		case eControlState::HOVERED: background = style.hover; break;
		case eControlState::FOCUSED: background = style.focus; break;
		case eControlState::HELD: background = style.pressed; break;
		case eControlState::DRAGGED: background = style.pressed; break;
	}

	m_background.SetColor(background.v);
	m_text.SetColor(style.text.v);
}

void Button::SetText(std::u32string text) {
	m_text.SetText(std::move(text));
}
const std::u32string& Button::GetText() const {
	return m_text.GetText();
}

float Button::SetDepth(float depth) {
	m_background.SetDepth(depth);
	m_text.SetDepth(depth + 0.1f);
	return 1.0f;
}

float Button::GetDepth() const {
	return m_background.GetDepth();
}


} // namespace inl::gui