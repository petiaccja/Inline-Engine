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
	CursorEvent() : cursorClientPos(0, 0) {}
	CursorEvent(Vector2i cursorClientPos) : cursorClientPos(cursorClientPos) {}

public:
	Vector2i cursorClientPos;
};

} //namespace inl::gui