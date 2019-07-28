#include "DebugInfoFrame.hpp"

#include <BaseLibrary/StringUtil.hpp>

#include <sstream>


DebugInfoFrame::DebugInfoFrame() {
	SetLayout(m_layout);
	m_layout.AddChild(m_videoCardLabel);
	m_layout.AddChild(m_frameBufferLabel);
	m_layout[&m_videoCardLabel].SetMargin({ 3, 3, 0, 0 }).SetAuto().MoveToBack();
	m_layout[&m_frameBufferLabel].SetMargin({ 3, 3, 0, 0 }).SetAuto().MoveToBack();
	m_layout.SetDirection(inl::gui::LinearLayout::eDirection::VERTICAL);
	m_layout.SetInverted(true);
	ShowBackground(false);

	inl::gui::ControlStyle textStyle = GetStyle();
	textStyle.text = { 1.0f, 1.0f, 1.0f };
	m_videoCardLabel.SetStyle(textStyle);
	m_frameBufferLabel.SetStyle(textStyle);
}


void DebugInfoFrame::Update(float elapsed) {
	Frame::Update(elapsed);
	const float coeff = 5.0f;
	const float t = std::exp(-coeff * elapsed);
	m_rollingAvgFrametime = m_rollingAvgFrametime * t + elapsed * (1 - t);

	std::stringstream ss;
	ss << "D3D12 -- "
	   << m_resolution.x << " x " << m_resolution.y << " -- "
	   << std::to_string(int(1 / m_rollingAvgFrametime)) << " FPS";
	m_frameBufferLabel.SetText(inl::EncodeString<char32_t>(ss.str()));
}


void DebugInfoFrame::UpdateStyle() {
	inl::gui::ControlStyle textStyle = GetStyle();
	textStyle.text = { 1.0f, 1.0f, 1.0f };
	m_videoCardLabel.SetStyle(textStyle);
	m_frameBufferLabel.SetStyle(textStyle);
}


void DebugInfoFrame::SetAdapterInfo(const inl::gxapi::AdapterInfo& info) {
	m_adatperInfo = info;

	std::stringstream ss;
	ss << info.name << " -- "
	   << (info.dedicatedVideoMemory / 1048576) << " MiB VRAM";
	m_videoCardLabel.SetText(inl::EncodeString<char32_t>(ss.str()));
}


void DebugInfoFrame::SetResolutionInfo(inl::Vec2u resolution) {
	m_resolution = resolution;

	std::stringstream ss;
}