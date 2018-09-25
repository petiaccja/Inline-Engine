#include "Button.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"



namespace inl::gui {


Button::Button() {
	m_text.reset(new PlaceholderTextEntity());
	m_background.reset(new PlaceholderOverlayEntity());
	m_text->SetZDepth(0.1f);
	m_background->SetZDepth(0.0f);
}

void Button::SetSize(Vec2u size) {
	m_background->SetScale(size);
	m_text->SetSize(size);
}

Vec2u Button::GetSize() const {
	return m_background->GetScale();
}

void Button::SetPosition(Vec2i position) {
	m_background->SetPosition(position);
	m_text->SetPosition(position);
}
Vec2i Button::GetPosition() const {
	return m_background->GetPosition();
}


void Button::Update(float elapsed) {
	return;
}


void Button::SetBackgroundColor(ColorF color) {
	m_background->SetColor(color.v);
}
ColorF Button::GetBackgroundColor() const {
	ColorF c;
	c.v = m_background->GetColor();
	return c;
}

void Button::SetTextColor(ColorF color) {
	m_text->SetColor(color.v);
}
ColorF Button::GetTextColor() const {
	ColorF c;
	c.v = m_text->GetColor();
	return c;
}

void Button::SetText(std::string text) {
	m_text->SetText(std::move(text));
}
const std::string& Button::GetText() const {
	return m_text->GetText();
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Button::GetTextEntities() {
	return { m_text };
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Button::GetOverlayEntities() {
	return { m_background };
}


} // namespace inl::gui