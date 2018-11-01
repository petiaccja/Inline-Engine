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
	m_text->SetColor(GetStyle().text.v);
}

void Label::SetHorizontalAlignment(float alignment) {
	m_text->SetHorizontalAlignment(alignment);
}


void Label::SetVerticalAlignment(float alignment) {
	m_text->SetVerticalAlignment(alignment);
}



void Label::SetText(std::u32string text) {
	m_text->SetText(std::move(text));
}
const std::u32string& Label::GetText() const {
	return m_text->GetText();
}

void Label::SetZOrder(int rank) {
	m_text->SetZDepth((float)rank);
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Label::GetTextEntities() {
	return { m_text };
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Label::GetOverlayEntities() {
	return { };
}


} // namespace inl::gui