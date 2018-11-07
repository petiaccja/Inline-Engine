#include "PortControl.hpp"


namespace inl::tool {


PortControl::PortControl(const NodeControl* node, int index)
	: m_node(node), m_index(index) {}


const NodeControl* PortControl::GetNode() const {
	return m_node;
}


int PortControl::GetPortIndex() const {
	return m_index;
}


} // namespace inl::tool
