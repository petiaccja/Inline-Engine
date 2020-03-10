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

Text::~Text() {
	if (m_context.scene) {
		m_context.scene->GetEntities<gxeng::ITextEntity>().Remove(m_entity.get());
	}
}


void Text::SetContext(const GraphicsContext& context) {
	assert(context.engine);
	assert(context.scene);

	if (m_context.scene) {
		auto& entitySet = m_context.scene->GetEntities<gxeng::ITextEntity>();
		if (entitySet.Contains(m_entity.get())) {
			entitySet.Remove(m_entity.get());
		}
	}

	std::unique_ptr<gxeng::ITextEntity> newEntity(context.engine->CreateTextEntity());

	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = context;
	if (IsShown()) {
		m_context.scene->GetEntities<gxeng::ITextEntity>().Add(m_entity.get());
	}
}


void Text::ClearContext() {
	if (m_context.scene) {
		m_context.scene->GetEntities<gxeng::ITextEntity>().Remove(m_entity.get());
	}

	auto newEntity = std::make_unique<PlaceholderTextEntity>();
	CopyProperties(*m_entity, *newEntity);

	m_entity = std::move(newEntity);
	m_context = { nullptr, nullptr };
}


void Text::SetClipRect(const RectF& rect, const Mat33& transform) {
	m_entity->SetAdditionalClip(rect, transform);
	m_entity->EnableAdditionalClip(true);
}


void Text::SetPostTransform(const Mat33& transform) {
	m_postTransform = transform;
	SetResultantTransform();
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
	m_position = position;
	SetResultantTransform();
}

Vec2 Text::GetPosition() const {
	return m_position;
}

float Text::SetDepth(float depth) {
	m_entity->SetZDepth(depth);
	return 1.0f;
}

float Text::GetDepth() const {
	return m_entity->GetZDepth();
}

bool Text::HitTest(const Vec2& point) const {
	return false;
}

void Text::UpdateStyle() {
	m_entity->SetFont(GetStyle().font);
	m_entity->SetColor(GetStyle().text.v);
}

void Text::Update(float elapsed) {
	bool shown = IsShown();
	if (m_context.scene) {
		auto& entitySet = m_context.scene->GetEntities<gxeng::ITextEntity>();
		bool contained = entitySet.Contains(m_entity.get());
		if (contained && !shown) {
			entitySet.Remove(m_entity.get());
		}
		if (!contained && shown) {
			entitySet.Add(m_entity.get());
		}
	}
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
	target.Transform() = source.Transform();
}

void Text::SetResultantTransform() {
	m_entity->Transform() = Mat33(Translation(m_position)) * m_postTransform;
}


} // namespace inl::gui