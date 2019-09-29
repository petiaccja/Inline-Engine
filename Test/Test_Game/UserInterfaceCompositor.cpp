#include "UserInterfaceCompositor.hpp"


UserInterfaceCompositor& TopLevelFrame::GetCompositor() const {
	assert(m_compositor != nullptr);
	return *m_compositor;
}


WindowLayout::Binding& TopLevelFrame::GetBinding() const {
	return GetCompositor().GetBinding(*this);
}


void TopLevelFrame::SetCompositor(UserInterfaceCompositor* compositor) {
	m_compositor = compositor;
}


UserInterfaceCompositor::UserInterfaceCompositor(inl::gui::Board& board) : m_board(board) {
	m_board.AddChild(m_layout);
}


void UserInterfaceCompositor::SetPosition(inl::Vec2 pos) {
	m_layout.SetPosition(pos);
}


void UserInterfaceCompositor::SetSize(inl::Vec2 size) {
	m_layout.SetSize(size);
}


inl::Vec2 UserInterfaceCompositor::GetPosition() const {
	return m_layout.GetPosition();
}


inl::Vec2 UserInterfaceCompositor::GetSize() const {
	return m_layout.GetSize();
}


WindowLayout::Binding& UserInterfaceCompositor::GetBinding(const TopLevelFrame& frame) {
	auto* ptr = dynamic_cast<const inl::gui::Control*>(&frame);
	return m_layout[ptr];
}
