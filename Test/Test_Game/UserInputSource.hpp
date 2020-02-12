#pragma once

#include "BaseLibrary/Platform/Input.hpp"

class ActionHeap;


class UserInputSource {
public:
	virtual void OnKeyboard(inl::KeyboardEvent evt, ActionHeap& actionHeap) {}
	virtual void OnMouseButton(inl::MouseButtonEvent evt, ActionHeap& actionHeap) {}
	virtual void OnMouseMove(inl::MouseMoveEvent evt, ActionHeap& actionHeap) {}
	virtual void OnMouseWheel(inl::MouseWheelEvent evt, ActionHeap& actionHeap) {}

	virtual void Emit(ActionHeap& actionHeap) {}

	void SetEnabled(bool enabled) { m_enabled = enabled; }
	void Enable() { m_enabled = true; }
	void Disable() { m_enabled = false; }
	bool IsEnabled() const { return m_enabled; }

private:
	bool m_enabled = true;
};
