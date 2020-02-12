#pragma once

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


class DebugInfoFrame : public inl::gui::Frame {
public:
	DebugInfoFrame();

	void Update(float elapsed) override;
	void UpdateStyle() override;

	void SetAdapterInfo(const inl::gxapi::AdapterInfo& info);
	void SetResolutionInfo(inl::Vec2u resolution);

private:
	inl::gui::LinearLayout m_layout;
	inl::gui::Label m_videoCardLabel;
	inl::gui::Label m_frameBufferLabel;
	float m_rollingAvgFrametime = 1.0f;
	inl::gxapi::AdapterInfo m_adatperInfo;
	inl::Vec2u m_resolution = { 0, 0 };
};
