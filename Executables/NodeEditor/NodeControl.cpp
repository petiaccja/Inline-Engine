#include "NodeControl.hpp"

#include <BaseLibrary/Platform/System.hpp>
#include <BaseLibrary/StringUtil.hpp>


namespace inl::tool {



NodeControl::NodeControl() {
	SetLayout(m_titleLayout);

	m_title.SetHorizontalAlignment(0.0f);

	m_titleLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_titleLayout.SetInverted(true);
	m_titleLayout.AddChild(m_title);
	m_titleLayout.AddChild(m_ioSplitLayout);
	m_titleLayout[&m_title].SetWidth(32);
	m_titleLayout[&m_ioSplitLayout].SetWeight(1.0f).SetMargin({ 0, 0, 0, 0 }).MoveToBack();

	m_ioSplitLayout.SetDirection(gui::LinearLayout::HORIZONTAL);
	m_ioSplitLayout.AddChild(m_inputPortsLayout);
	m_ioSplitLayout.AddChild(m_outputPortsLayout);
	m_ioSplitLayout[&m_inputPortsLayout].SetWeight(1.0f).SetMargin({ 0, 0, 0, 0 });
	m_ioSplitLayout[&m_outputPortsLayout].SetWeight(1.0f).SetMargin({ 0, 0, 0, 0 });

	m_inputPortsLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_inputPortsLayout.SetInverted(true);
	m_outputPortsLayout.SetDirection(gui::LinearLayout::VERTICAL);
	m_outputPortsLayout.SetInverted(true);

	UpdateHeight();

	m_title.OnEnterArea += [](Control*) {
		System::SetCursorVisual(eCursorVisual::SIZEALL, nullptr);
	};
	m_title.OnDragBegin += [this](Control*, Vec2 dragStart) {
		CallEventUpstream(&Control::OnDragBegin, this, dragStart);
	};
	m_title.OnDrag += [this](Control*, Vec2 dragPos) {
		CallEventUpstream(&Control::OnDrag, this, dragPos);
	};
	m_title.OnDragEnd += [this](Control*, Vec2 dragEnd, Control* target) {
		CallEventUpstream(&Control::OnDragEnd, this, dragEnd, target);
	};
}

void NodeControl::SetName(std::string name) {
	m_name = name;
	UpdateTitle();
}


void NodeControl::SetType(std::string type) {
	m_type = type;
	UpdateTitle();
}


void NodeControl::SetInputPorts(std::vector<std::pair<std::string, std::string>> inputPorts) {
	m_inputPortsLayout.ClearChildren();
	m_inputPorts.clear();

	// Reserve avoid reallocation, which is important because items must keep
	// their memory address for the accomodating layout to work..
	m_inputPorts.reserve(inputPorts.size());

	for (auto& desc : inputPorts) {
		PortControl& port = m_inputPorts.emplace_back(this, (int)m_inputPorts.size(), true);
		port.SetText(EncodeString<char32_t>(desc.first + " : " + desc.second));
		m_inputPortsLayout.AddChild(port);
		m_inputPortsLayout[&port].SetWidth(26).MoveToBack();
	}

	UpdateHeight();
}


void NodeControl::SetOutputPorts(std::vector<std::pair<std::string, std::string>> outputPorts) {
	m_outputPortsLayout.ClearChildren();
	m_outputPorts.clear();

	m_outputPorts.reserve(outputPorts.size());

	for (auto& desc : outputPorts) {
		PortControl& port = m_outputPorts.emplace_back(this, (int)m_outputPorts.size(), false);
		port.SetText(EncodeString<char32_t>(desc.first + " : " + desc.second));
		m_outputPortsLayout.AddChild(port);
		m_outputPortsLayout[&port].SetWidth(26).MoveToBack();
	}

	UpdateHeight();
}


const PortControl& NodeControl::GetInputPort(int index) const {
	return m_inputPorts[index];
}


const PortControl& NodeControl::GetOutputPort(int index) const {
	return m_outputPorts[index];
}


void NodeControl::UpdateTitle() {
	m_title.SetText(EncodeString<char32_t>(m_name + " : " + m_type));
}

void NodeControl::UpdateHeight() {
	float height = 0.0f;
	height += m_titleLayout[&m_title].GetValue();
	height += 26 * std::max(m_inputPorts.size(), m_outputPorts.size());
	SetSize({ GetSize().x, height });
}


} // namespace inl::tool
