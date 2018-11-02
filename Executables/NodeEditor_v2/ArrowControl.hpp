#pragma once


#include <InlineMath.hpp>
#include <GuiEngine/StandardControl.hpp>
#include <GraphicsEngine/Scene/IOverlayEntity.hpp>

#include <memory>


namespace inl::tool {


class ArrowControl : public gui::StandardControl {
public:
    void SetEndPoints(Vec2 p1, Vec2 p2);
    std::pair<Vec2, Vec2> GetEndPoints();

    void SetLineWidth(float width);
    float GetLineWidth(float width);
protected:
    // Use SetEndPoints.
    void SetPosition(Vec2i position) override;

private:
    std::unique_ptr<gxeng::IOverlayEntity> m_bezierLine;
    std::unique_ptr<gxeng::IOverlayEntity> m_arrowHead;
    std::unique_ptr<gxeng::IOverlayEntity> m_holdPoint;
};


} // namespace inl::tool