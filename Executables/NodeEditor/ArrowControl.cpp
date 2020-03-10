#include "ArrowControl.hpp"

#include <GuiEngine/Placeholders/PlaceholderOverlayEntity.hpp>

namespace inl::tool {


ArrowControl::ArrowControl() {
	m_holdPoint.SetSize({ 6, 6 });
	m_arrowHead1.SetSize({ 12, 1 });
	m_arrowHead2.SetSize({ 12, 1 });
	//AddChild(m_bezierLine);
	AddChild(m_holdPoint);
	AddChild(m_arrowHead1);
	AddChild(m_arrowHead2);

	for (auto& segment : m_bezierSections) {
		AddChild(segment);
	}
}


void ArrowControl::SetEndPoints(Vec2 p1, Vec2 p2) {
	m_p1 = p1;
	m_p2 = p2;

	m_holdPoint.SetPosition((p1 + p2) / 2.f);

	m_arrowHead1.SetPosition(p2 + Vec2(-5.f, 3.5f));
	m_arrowHead1.SetRotation(-Constants<float>::Pi / 6);
	m_arrowHead2.SetPosition(p2 + Vec2(-5.f, -3.5f));
	m_arrowHead2.SetRotation(+Constants<float>::Pi / 6);

	Vec2 dir = p2 - p1;
	float leaning = std::min(100.f, dir.x);
	if (dir.x < 25.f) {
		leaning = std::max(0.0f, -dir.x) / 3.5f + 25.f;
	}


	m_bezierCurve.p[0] = p1;
	m_bezierCurve.p[1] = p1 + Vec2{ leaning, 0.0f };
	m_bezierCurve.p[2] = p2 - Vec2{ leaning, 0.0f };
	m_bezierCurve.p[3] = p2;

	size_t numSegments = m_bezierSections.size();
	for (size_t i = 0; i < numSegments; ++i) {
		float tBegin = (float)i / (float)numSegments;
		float tEnd = (float)(i + 1) / (float)numSegments;
		Vec2 pBegin = m_bezierCurve(tBegin);
		Vec2 pEnd = m_bezierCurve(tEnd);

		m_bezierSections[i].SetPosition((pBegin + pEnd) / 2.f);
		Vec2 diff = pEnd - pBegin;
		m_bezierSections[i].SetSize({ Length(diff), GetLineWidth() });
		float angle = atan2(diff.y, diff.x);
		m_bezierSections[i].SetRotation(angle);
	}
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
	m_arrowHead1.SetDepth(depth + 0.1f);
	m_arrowHead2.SetDepth(depth + 0.1f);
	for (auto& segment : m_bezierSections) {
		segment.SetDepth(depth + 0.1f);
	}

	return 1.0f;
}


float ArrowControl::GetDepth() const {
	return m_bezierLine.GetDepth();
}

void ArrowControl::Update(float elapsed) {
	m_bezierLine.SetColor(GetStyle().text.v);
	m_holdPoint.SetColor(GetStyle().text.v);
	m_arrowHead1.SetColor(GetStyle().text.v);
	m_arrowHead2.SetColor(GetStyle().text.v);
	for (auto& segment : m_bezierSections) {
		segment.SetColor(GetStyle().text.v);
	}
}

bool ArrowControl::HitTest(const Vec2& point) const {
	RectF catchRect = RectF::FromCenter((m_p1 + m_p2) / 2.f, { 4, 4 });
	return catchRect.IsPointInside(point);
}


} // namespace inl::tool
