#include "Button.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"

#include <BaseLibrary/Platform/System.hpp>


namespace inl::gui {


Button::Button() {
	OnEnterArea += [](auto...) {
		System::SetCursorVisual(eCursorVisual::ARROW, nullptr);
	};
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
		return { m_text.CalculateTextHeight(), m_text.CalculateTextWidth() };
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
	//ColorF foreground;
	//switch (GetState()) {
	//	case eStandardControlState::DEFAULT: foreground = GetStyle().foreground; break;
	//	case eStandardControlState::MOUSEOVER: foreground = GetStyle().hover; break;
	//	case eStandardControlState::FOCUSED: foreground = GetStyle().focus; break;
	//	case eStandardControlState::PRESSED: foreground = GetStyle().pressed; break;
	//}
	//m_background->SetColor(foreground.v);
	//m_text->SetColor(GetStyle().text.v);
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


void Button::SetVisible(bool visible) {
	throw NotImplementedException();
}


bool Button::GetVisible() const {
	throw NotImplementedException();
}


bool Button::IsShown() const {
	throw true;
}


} // namespace inl::gui