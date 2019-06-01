#pragma once


#include <GuiEngine/Sprite.hpp>
#include <GuiEngine/Control.hpp>

#include <InlineMath.hpp>


namespace inl::tool {


class ArrowControl : public gui::Control {
public:
	ArrowControl();

	void SetEndPoints(Vec2 p1, Vec2 p2);
	std::pair<Vec2, Vec2> GetEndPoints() const;

	void SetLineWidth(float width);
	float GetLineWidth() const;

	float SetDepth(float depth) override;
	float GetDepth() const override;

	void Update(float elapsed) override;

	Vec2 GetSize() const override {
		auto span = m_p1 - m_p2;
		span = { std::abs(span.x) + 1000.f, std::abs(span.y) + 400.f };
		return span;
	}
	Vec2 GetMinimumSize() const override { return { 0,0 }; }
	Vec2 GetPreferredSize() const override { return GetSize(); }
	Vec2 GetPosition() const override { return (m_p1 + m_p2) / 2.f; }

	bool HitTest(const Vec2& point) const override;
protected:
	// Use SetEndPoints.
	void SetSize(const Vec2& size) override {}
	void SetPosition(const Vec2& position) override {}

private:
	std::array<gui::Sprite, 32> m_bezierSections;
	BezierCurve<float, 2, 3> m_bezierCurve;
	gui::Sprite m_bezierLine;
	gui::Sprite m_arrowHead1;
	gui::Sprite m_arrowHead2;
	gui::Sprite m_holdPoint;
	Vec2 m_p1, m_p2;
};


} // namespace inl::tool