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

void Label::SetSize(Vec2 size) {
	m_text->SetSize(size);
}

Vec2 Label::GetSize() const {
	return m_text->GetSize();
}

Vec2 Label::GetPreferredSize() const {
	if (m_text->GetFont()) {
		return { std::ceil(m_text->CalculateTextWidth()), std::ceil(m_text->CalculateTextHeight()) };
	}
	else {
		return { 10, 10 };
	}
}


Vec2 Label::GetMinimumSize() const {
	return { 0.0f, 0.0f };
}


void Label::SetPosition(Vec2 position) {
	m_text->SetPosition(position);
}
Vec2 Label::GetPosition() const {
	return m_text->GetPosition();
}


void Label::Update(float elapsed) {
	UpdateClip();

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

float Label::SetDepth(float depth) {
	m_text->SetZDepth(depth);
	return 1.0f;
}

float Label::GetDepth() const {
	return m_text->GetZDepth();
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> Label::GetTextEntities() {
	return { m_text };
}

std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> Label::GetOverlayEntities() {
	return { };
}


} // namespace inl::gui