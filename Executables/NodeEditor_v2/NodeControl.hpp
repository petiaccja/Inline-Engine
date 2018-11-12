#pragma once

#include "PortControl.hpp"

#include <GuiEngine/Button.hpp>
#include <GuiEngine/Frame.hpp>
#include <GuiEngine/Label.hpp>
#include <GuiEngine/LinearLayout.hpp>


namespace inl::tool {


class NodeControl : public gui::Frame {
public:
	NodeControl();

	void SetName(std::string name);
	void SetType(std::string type);
	void SetInputPorts(std::vector<std::pair<std::string, std::string>> inputPorts);
	void SetOutputPorts(std::vector<std::pair<std::string, std::string>> outputPorts);

	const PortControl& GetInputPort(int index) const;
	const PortControl& GetOutputPort(int index) const;
private:
	void UpdateTitle();
	void UpdateHeight();

private:
	gui::Label m_title;

	gui::LinearLayout m_titleLayout;
	gui::LinearLayout m_ioSplitLayout;
	gui::LinearLayout m_inputPortsLayout;
	gui::LinearLayout m_outputPortsLayout;

	std::vector<PortControl> m_inputPorts;
	std::vector<PortControl> m_outputPorts;

	std::string m_name;
	std::string m_type;
};


} // namespace inl::tool
