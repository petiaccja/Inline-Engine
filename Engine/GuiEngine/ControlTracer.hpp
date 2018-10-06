#pragma once

#include <BaseLibrary/Platform/Input.hpp>


namespace inl::gui {

class Control;
class Frame;



class ControlTracer {
public:

	void SetFrame(Frame* frame);
	Frame* GetFrame() const;

	void OnMouseButton(MouseButtonEvent evt);
	void OnMouseMove(MouseMoveEvent evt);
	void OnKeyboard(KeyboardEvent evt);

private:
	Control* HitTestRecurse(Vec2 point, Control* top);
	bool HitTest(Vec2 point, Control* control);

private:
	Frame* m_frame = nullptr;
};


} // namespace inl::gui