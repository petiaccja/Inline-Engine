#pragma once

#include <GuiEngine/Button.hpp>


namespace inl::tool {

class NodeControl;


class PortControl : public gui::Button {
public:
	PortControl(const NodeControl* node, int index);

	const NodeControl* GetNode() const;
	int GetPortIndex() const;
private:
	const NodeControl* const m_node = nullptr;
	const int m_index = 0;
};


} // namespace inl::tool