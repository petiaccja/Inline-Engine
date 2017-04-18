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
	CursorEvent() : cursorContentPos(0, 0), mouseDelta(0,0){}
	CursorEvent(Vector2i cursorContentPos) : cursorContentPos(cursorContentPos), mouseDelta(0,0) {}

public:
	Vector2i cursorContentPos;
	Vector2i mouseDelta;
};

} //namespace inl::gui