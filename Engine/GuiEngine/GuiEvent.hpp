#pragma once
#include "BaseLibrary\Common.hpp"
#include "BaseLibrary\Platform\Sys.hpp"

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
	CursorEvent() : cursorPos(0, 0), mouseDelta(0,0), mouseButton(eMouseBtn::INVALID){}
	CursorEvent(Vec2 CursorPosContentSpace) : cursorPos(CursorPosContentSpace), mouseDelta(0,0) {}

public:
	Vec2 cursorPos;
	Vec2 mouseDelta;
	eMouseBtn mouseButton;
};

} //namespace inl::gui