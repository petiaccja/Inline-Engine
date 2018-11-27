#pragma once

#include "Layout.hpp"


namespace inl::gui {


void Layout::SetVisible(bool visible) {
	// empty
}


bool Layout::GetVisible() const {
	return false;
}

bool Layout::IsShown() const {
	if (m_parent) {
		return m_parent->IsShown();
	}
	return false;
}


void Layout::SetStyle(nullptr_t) {
	m_isStyleInherited = true;
	if (m_parent) {
		m_style = m_parent->GetStyle();
	}
}


void Layout::SetStyle(const ControlStyle& style, bool asDefault) {
	m_style = style;
	m_isStyleInherited = asDefault;
}


const ControlStyle& Layout::GetStyle() const {
	return m_style;
}

Control* Layout::GetParent() const {
	return m_parent;
}


void Layout::OnAttach(Control* parent) {
	m_parent = parent;
	m_context = Control::GetContext(parent);
	if (m_parent) {
		m_style = m_parent->GetStyle();
	}
}

void Layout::OnDetach() {
	m_context = nullptr;
	m_parent = nullptr;
}


} // namespace inl::gui
