#include "ArrowControl.hpp"

#include <GuiEngine/Placeholders/PlaceholderOverlayEntity.hpp>

namespace inl::tool {


ArrowControl::ArrowControl() {
	m_holdPoint.SetSize({ 6, 6 });
	m_arrowHead.SetSize({ 10,10 });
	AddChild(m_bezierLine);
	AddChild(m_holdPoint);
	AddChild(m_arrowHead);
}


void ArrowControl::SetEndPoints(Vec2 p1, Vec2 p2) {
	m_p1 = p1;
	m_p2 = p2;
	Vec2 center = (p1 + p2) / 2;
	Vec2 diff = p2 - p1;

	m_bezierLine.SetPosition(center);
	m_bezierLine.SetSize({ diff.Length(), GetLineWidth() });
	float angle = atan2(diff.y, diff.x);
	m_bezierLine.SetRotation(angle);

	m_holdPoint.SetPosition(center);
	m_holdPoint.SetRotation(angle);

	m_arrowHead.SetPosition(p2 - 5.f * diff.Normalized());
	m_arrowHead.SetRotation(angle + Constants<float>::Pi / 4);
}


std::pair<Vec2, Vec2> ArrowControl::GetEndPoints() const {
	return { m_p1, m_p2 };
}


void ArrowControl::SetLineWidth(float width) {
	m_bezierLine.SetSize({ m_bezierLine.GetSize().x, width });
}


float ArrowControl::GetLineWidth() const {
	return m_bezierLine.GetSize().y;
}


float ArrowControl::SetDepth(float depth) {
	m_bezierLine.SetDepth(depth);
	m_holdPoint.SetDepth(depth + 0.1f);
	m_arrowHead.SetDepth(depth + 0.1f);
	return 1.0f;
}


float ArrowControl::GetDepth() const {
	return m_bezierLine.GetDepth();
}

void ArrowControl::Update(float elapsed) {
	//m_bezierLine.SetColor(GetStyle().text.v);
	//m_holdPoint.SetColor(GetStyle().text.v);
	//m_arrowHead.SetColor(GetStyle().text.v);
}


} // namespace inl::tool
