#pragma once

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


namespace inl {
class Window;
}


class DebugInfoFrame : public gui::Frame {
public:
	DebugInfoFrame();

	void Update(float elapsed) override;
	void UpdateStyle() override;

	void SetAdapterInfo(const gxapi::AdapterInfo& info);
	void SetResolutionInfo(Vec2u resolution);

private:
	gui::LinearLayout m_layout;
	gui::Label m_videoCardLabel;
	gui::Label m_frameBufferLabel;
	float m_rollingAvgFrametime = 1.0f;
	gxapi::AdapterInfo m_adatperInfo;
	Vec2u m_resolution = { 0, 0 };
};
