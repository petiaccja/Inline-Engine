#pragma once

#include "UserInputSource.hpp"

class ActionHeap;


class CameraMoveSource : public UserInputSource {
public:
	void OnKeyboard(inl::KeyboardEvent evt, ActionHeap& actionHeap) override;
	void OnMouseMove(inl::MouseMoveEvent evt, ActionHeap& actionHeap) override;
	void OnMouseButton(inl::MouseButtonEvent evt, ActionHeap& actionHeap) override;
	void Emit(ActionHeap& actionHeap) override;

private:
	bool m_movingForward = false;
	bool m_movingLeft = false;
	bool m_movingBack = false;
	bool m_movingRight = false;
	bool m_movingDown = false;
	bool m_movingUp = false;
	bool m_rotateEnabled = false;
	bool m_boost = false;
};
