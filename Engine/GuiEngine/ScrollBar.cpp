#include "ScrollBar.hpp"
#include "Placeholders/PlaceholderOverlayEntity.hpp"


namespace inl::gui {


ScrollBar::ScrollBar(eDirection direction) {
	m_direction = direction;
	m_background = std::make_unique<PlaceholderOverlayEntity>();
	m_handle = std::make_unique<PlaceholderOverlayEntity>();

	SetScripts();
}


void ScrollBar::SetSize(Vec2 size) {
	m_background->SetScale(size);
}


Vec2 ScrollBar::GetSize() const {
	return m_background->GetScale();
}


Vec2 ScrollBar::GetMinimumSize() const {
	return { 0, 0 };
}


Vec2 ScrollBar::GetPreferredSize() const {
	return { 10.f, 10.f };
}


void ScrollBar::SetPosition(Vec2 position) {
	m_background->SetPosition(position);
}


Vec2 ScrollBar::GetPosition() const {
	return m_background->GetPosition();
}


void ScrollBar::Update(float elapsed) {
	UpdateHandlePosition();
	UpdateColor();
}


float ScrollBar::SetDepth(float depth) {
	m_background->SetZDepth(depth);
	m_handle->SetZDepth(depth + 0.1f);
	return 1.0f;
}


float ScrollBar::GetDepth() const {
	return m_background->GetZDepth();
}


void ScrollBar::SetDirection(eDirection direction) {
	m_direction = direction;
}


void ScrollBar::SetInverted(bool inverted) {
	m_inverted = inverted;
}


void ScrollBar::SetTotalLength(float length) {
	m_totalLength = length;
}


void ScrollBar::SetVisibleLength(float length) {
	m_visibleLength = length;
}


void ScrollBar::SetVisiblePosition(float begin) {
	m_visiblePosition = begin;
}


void ScrollBar::SetHandleMinimumSize(float size) {
	m_minHandleSize = size;
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> ScrollBar::GetTextEntities() {
	return {};
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> ScrollBar::GetOverlayEntities() {
	return { m_background, m_handle };
}


void ScrollBar::UpdateHandlePosition() {
	float lengthFrac = m_visibleLength / m_totalLength;
	float posBeginFrac = m_visiblePosition / m_totalLength;
	float posEndFrac = posBeginFrac + lengthFrac;
	float posCenterFrac = (posBeginFrac + posEndFrac) / 2.0f;
	if (m_inverted) {
		posCenterFrac = 1.0f - posCenterFrac;
	}

	float primaryBudget = m_direction == HORIZONTAL ? GetSize().x : GetSize().y;
	float auxBudget = m_direction == HORIZONTAL ? GetSize().y : GetSize().x;
	primaryBudget = std::max(primaryBudget - 4.0f, 0.0f);
	auxBudget = std::max(auxBudget - 4.0f, 0.0f);

	float primaryPos = primaryBudget * posCenterFrac;
	float primarySize = primaryBudget * lengthFrac;
	float auxSize = auxBudget;
	float auxPos = 0.0f;

	Vec2 pos;
	Vec2 size;
	if (m_direction == HORIZONTAL) {
		Vec2 base = GetPosition() - Vec2(primaryBudget / 2.0f, 0.0f);
		pos = base + Vec2(primaryPos, auxPos);
		size = Vec2(primarySize, auxSize);
	}
	else {
		Vec2 base = GetPosition() - Vec2(0.0f, primaryBudget / 2.0f);
		pos = base + Vec2(auxPos, primaryPos);
		size = Vec2(auxSize, primarySize);
	}

	m_handle->SetPosition(pos);
	m_handle->SetScale(size);	
}


void ScrollBar::UpdateColor() {
	// TODO
	m_background->SetColor(GetStyle().foreground.v);
	m_handle->SetColor(GetStyle().background.v);
}


void ScrollBar::SetScripts() {
	
}


}
