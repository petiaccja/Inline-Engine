#pragma once
#include "BaseLibrary/Common.hpp"
#include "BaseLibrary/Platform/Input.hpp"

#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max

#include "GuiRect.hpp"

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
	CursorEvent() : cursorPos(0, 0), cursorDelta(0,0), mouseButton((eMouseButton)0){}
	CursorEvent(Vec2 CursorPosContentSpace) : cursorPos(CursorPosContentSpace), cursorDelta(0,0) {}

public:
	Vec2 cursorPos;
	Vec2 cursorDelta;
	eMouseButton mouseButton;
};

struct UpdateEvent
{
	float deltaTime;
};

struct TransformEvent
{
	GuiRectF rect;
};

struct PositionEvent
{
	Vec2 pos;
};

struct SizeEvent
{
	Vec2 size;
};

struct ParentEvent
{
	class Gui* parent;
};

struct ChildEvent
{
	class Gui* child;
};

struct PaintEvent
{
	Gdiplus::Graphics* graphics;
};

} //namespace inl::gui