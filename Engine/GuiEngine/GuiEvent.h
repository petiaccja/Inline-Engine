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
	CursorEvent(ivec2 cursorClientPos) : cursorClientPos(cursorClientPos) {}

public:
	ivec2 cursorClientPos;
};

} //namespace inl::gui