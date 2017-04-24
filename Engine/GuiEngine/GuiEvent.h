#pragma once
#include <BaseLibrary\Common_tmp.hpp>

namespace inl::gui {

enum class eEventPropagationPolicy
{
	PROCESS,
	AVOID,
	STOP,
	PROCESS_STOP,
};

class CursorEvent
{
public:
	CursorEvent() : cursorPos(0, 0), mouseDelta(0,0){}
	CursorEvent(Vector2f CursorPosContentSpace) : cursorPos(CursorPosContentSpace), mouseDelta(0,0) {}

public:
	Vector2f cursorPos;
	Vector2f mouseDelta;
};

} //namespace inl::gui