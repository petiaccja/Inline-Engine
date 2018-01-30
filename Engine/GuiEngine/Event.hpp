#pragma once
#include "BaseLibrary/Platform/Input.hpp"

#define min(a,b) a < b ? a : b
#define max(a,b) a > b ? a : b
#include <gdiplus.h>
#undef min
#undef max

#include "Rect.hpp"


namespace inl::gui {

class Gui;

enum class eEventPropagationPolicy
{
	PROCESS,
	AVOID,
	STOP,
	PROCESS_STOP,
};

struct GuiEvent
{
	GuiEvent(): self(nullptr), target(nullptr), m_bStopPropagation(false){}

	void StopPropagation() { m_bStopPropagation = true; }
	bool IsPropagationStopped() { return m_bStopPropagation; }

	Gui* self;
	Gui* target;
private:
	bool m_bStopPropagation : 1;
};

struct DragDropEvent : GuiEvent
{
	std::string text;
	std::vector<std::experimental::filesystem::path> filePaths;
};

struct CursorEvent : GuiEvent
{
public:
	CursorEvent() : cursorPos(0, 0), cursorDelta(0,0), mouseButton((eMouseButton)0){}
	CursorEvent(Vec2 CursorPosContentSpace): cursorPos(CursorPosContentSpace), cursorDelta(0,0) {}

public:
	Vec2 cursorPos;
	Vec2 cursorDelta;
	eMouseButton mouseButton;
};

struct UpdateEvent : GuiEvent
{
	float deltaTime;
};

struct TransformEvent : GuiEvent
{
	GuiRectF rect;
};

struct PositionEvent : GuiEvent
{
	Vec2 pos;
};

struct SizeEvent : GuiEvent
{
	Vec2 size;
};

struct ParentEvent : GuiEvent
{
	class Gui* parent;
};

struct ChildEvent : GuiEvent
{
	class Gui* child;
};

struct PaintEvent : GuiEvent
{
	Gdiplus::Graphics* graphics;
};

} //namespace inl::gui
