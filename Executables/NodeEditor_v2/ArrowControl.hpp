#pragma once


#include <GraphicsEngine/Scene/IOverlayEntity.hpp>
#include <GuiEngine/StandardControl.hpp>

#include <InlineMath.hpp>
#include <memory>


namespace inl::tool {


class ArrowControl : public gui::StandardControl {
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
	void SetSize(Vec2 size) override {}

	void SetPosition(Vec2 position) override {}

	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::ITextEntity>>> GetTextEntities() override;
	std::vector<std::reference_wrapper<std::unique_ptr<gxeng::IOverlayEntity>>> GetOverlayEntities() override;

private:
	std::unique_ptr<gxeng::IOverlayEntity> m_bezierLine;
	std::unique_ptr<gxeng::IOverlayEntity> m_arrowHead;
	std::unique_ptr<gxeng::IOverlayEntity> m_holdPoint;
	Vec2 m_p1, m_p2;
};


} // namespace inl::tool