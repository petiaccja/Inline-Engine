#pragma once

#include <GuiEngine/Button.hpp>


namespace inl::tool {

class NodeControl;


class PortControl : public gui::Button {
public:
	PortControl(const NodeControl* node, int index, bool input);

	const NodeControl* GetNode() const;
	int GetPortIndex() const;
	bool IsInput() const { return m_input; }
	bool IsOutput() const { return !IsInput(); }
private:
	const NodeControl* const m_node = nullptr;
	const int m_index = 0;
	const bool m_input = true;
};


} // namespace inl::tool