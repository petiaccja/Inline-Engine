#include "Text.hpp"

#include "Placeholders/PlaceholderTextEntity.hpp"

namespace inl::gui {


Text::Text() {
	m_entity = std::make_unique<PlaceholderTextEntity>();
}


Text::Text(const Text& other) {
	m_entity = std::make_unique<PlaceholderTextEntity>();

	assert(other.m_entity);
	CopyProperties(*other.m_entity, *m_entity);
}


Text::Text(Text&& other) noexcept {
	m_entity = std::move(other.m_entity);
	m_context = other.m_context;
}


void Text::SetContext(const GraphicsContext& context) {
	assert(context.engine);
	assert(context.scene);

	std::unique_ptr<gxeng::ITextEntity> newEntity(context.engine->CreateTextEntity());

	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = context;
}


void Text::ClearContext() {
	auto newEntity = std::make_unique<PlaceholderTextEntity>();
	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = { nullptr, nullptr };
}


//-------------------------------------
// Control
//-------------------------------------

void Text::SetSize(const Vec2& size) {
	m_entity->SetSize(size);
}

Vec2 Text::GetSize() const {
	return m_entity->GetSize();
}

Vec2 Text::GetPreferredSize() const {
	return { m_entity->CalculateTextWidth(), m_entity->CalculateTextHeight() };
}

Vec2 Text::GetMinimumSize() const {
	return { 0, 0 };
}

void Text::SetPosition(const Vec2& position) {
	m_entity->SetPosition(position);
}

Vec2 Text::GetPosition() const {
	return m_entity->GetPosition();
}

void Text::SetVisible(bool visible) {
	throw NotImplementedException();
}

bool Text::GetVisible() const {
	throw NotImplementedException();
}

bool Text::IsShown() const {
	return true;
}


void Text::CopyProperties(const gxeng::ITextEntity& source, gxeng::ITextEntity& target) {
	target.SetFont(source.GetFont());
	target.SetFontSize(source.GetFontSize());
	target.SetText(source.GetText());
	target.SetColor(source.GetColor());
	target.SetSize(source.GetSize());
	auto [rect, transform] = source.GetAdditionalClip();
	target.SetAdditionalClip(rect, transform);
	target.EnableAdditionalClip(source.IsAdditionalClipEnabled());
	target.SetHorizontalAlignment(source.GetHorizontalAlignment());
	target.SetVerticalAlignment(source.GetVerticalAlignment());
	target.SetZDepth(source.GetZDepth());
	target.SetTransform(source.GetTransform());
}


} // namespace inl::gui