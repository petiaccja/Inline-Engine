#pragma once


#include <GuiEngine/Sprite.hpp>
#include <GuiEngine/Control.hpp>

#include <InlineMath.hpp>
#include <memory>


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

	Vec2 GetSize() const override { return { 6,6 }; }
	Vec2 GetMinimumSize() const override { return { 0,0 }; }
	Vec2 GetPreferredSize() const override { return GetSize(); }
	Vec2 GetPosition() const override { return (m_p1 + m_p2) / 2.f; }
protected:
	// Use SetEndPoints.
	void SetSize(const Vec2& size) override {}
	void SetPosition(const Vec2& position) override {}

private:
	gui::Sprite m_bezierLine;
	gui::Sprite m_arrowHead;
	gui::Sprite m_holdPoint;
	Vec2 m_p1, m_p2;
};


} // namespace inl::tool