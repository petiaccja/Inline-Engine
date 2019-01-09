#include "ArrowControl.hpp"

#include <GuiEngine/Placeholders/PlaceholderOverlayEntity.hpp>

namespace inl::tool {


ArrowControl::ArrowControl()
	: m_bezierLine(std::make_unique<gui::PlaceholderOverlayEntity>()),
	  m_holdPoint(std::make_unique<gui::PlaceholderOverlayEntity>()),
	  m_arrowHead(std::make_unique<gui::PlaceholderOverlayEntity>()) {
	m_holdPoint->SetScale({ 6, 6 });
	m_arrowHead->SetScale({ 10,10 });
}


void ArrowControl::SetEndPoints(Vec2 p1, Vec2 p2) {
	m_p1 = p1;
	m_p2 = p2;
	Vec2 center = (p1 + p2) / 2;
	Vec2 diff = p2 - p1;

	m_bezierLine->SetPosition(center);
	m_bezierLine->SetScale({ diff.Length(), GetLineWidth() });
	float angle = atan2(diff.y, diff.x);
	m_bezierLine->SetRotation(angle);

	m_holdPoint->SetPosition(center);
	m_holdPoint->SetRotation(angle);

	m_arrowHead->SetPosition(p2 - 5.f * diff.Normalized());
	m_arrowHead->SetRotation(angle + Constants<float>::Pi / 4);
}


std::pair<Vec2, Vec2> ArrowControl::GetEndPoints() const {
	return { m_p1, m_p2 };
}


void ArrowControl::SetLineWidth(float width) {
	m_bezierLine->SetScale({ m_bezierLine->GetScale().x, width });
}


float ArrowControl::GetLineWidth() const {
	return m_bezierLine->GetScale().y;
}


float ArrowControl::SetDepth(float depth) {
	m_bezierLine->SetZDepth(depth);
	m_holdPoint->SetZDepth(depth + 0.1f);
	m_arrowHead->SetZDepth(depth + 0.1f);
	return 1.0f;
}


float ArrowControl::GetDepth() const {
	return m_bezierLine->GetZDepth();
}

void ArrowControl::Update(float elapsed) {
	UpdateClip();
	m_bezierLine->SetColor(GetStyle().text.v);
	m_holdPoint->SetColor(GetStyle().text.v);
	m_arrowHead->SetColor(GetStyle().text.v);
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> ArrowControl::GetTextEntities() {
	return {};
}


std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> ArrowControl::GetOverlayEntities() {
	return { m_bezierLine, m_arrowHead, m_holdPoint };
}



} // namespace inl::tool
