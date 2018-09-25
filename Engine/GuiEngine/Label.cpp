#include "Label.hpp"

#include "Layout.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"
#include "Placeholders/PlaceholderTextEntity.hpp"



namespace inl::gui {


Label::Label() {
	m_text.reset(new PlaceholderTextEntity());
	m_text->SetZDepth(0.1f);
	m_text->SetHorizontalAlignment(-1);
}

void Label::SetSize(Vec2u size) {
	m_text->SetSize(size);
}

Vec2u Label::GetSize() const {
	return m_text->GetSize();
}

void Label::SetPosition(Vec2i position) {
	m_text->SetPosition(position);
}
Vec2i Label::GetPosition() const {
	return m_text->GetPosition();
}


void Label::Update(float elapsed) {
	return;
}

void Label::SetTextColor(ColorF color) {
	m_text->SetColor(color.v);
}
ColorF Label::GetTextColor() const {
	ColorF c;
	c.v = m_text->GetColor();
	return c;
}

void Label::SetText(std::string text) {
	m_text->SetText(std::move(text));
}
const std::string& Label::GetText() const {
	return m_text->GetText();
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Label::GetTextEntities() {
	return { m_text };
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Label::GetOverlayEntities() {
	return { };
}


} // namespace inl::gui