#pragma once
#include <BaseLibrary/Platform/Sys.hpp>
#include <array>
#include <functional>
#include <queue>

#include <BaseLibrary/Delegate.hpp>

namespace inl::core {

class InputCore
{
public:
	InputCore();

public:
	void SimulateKeyPress(eKey key);
	void SimulateKeyRelease(eKey key);

	void SimulateMouseBtnPress(eMouseButton button);
	void SimulateMouseBtnRelease(eMouseButton button);
	void SimulateMouseMove(const Vec2i& mouseDelta, const Vec2i& clientCursorPos);

	void BindKeyDown(eKey key, const std::function<void()>& function);
	void BindKeyPress(eKey key, const std::function<void()>& function);
	void BindKeyRelease(eKey key, const std::function<void()>& function);

	void BindMousePress(eMouseButton button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseRelease(eMouseButton button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseDown(eMouseButton button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseMove(const std::function<void(const Vec2i& mouseDelta, const Vec2i& clientCursorPos)>& function);

	void ClearFrameData();
	void Update();

	bool IsKeyDown(eKey key);
	bool IsKeyPressed(eKey key);
	bool IsKeyReleased(eKey key);

	bool IsMousePressed(eMouseButton eButton);
	bool IsMouseReleased(eMouseButton eButton);
	bool IsMouseDown(eMouseButton eButton);
	bool IsMouseClicked(eMouseButton eButton);

	Vec2i GetCursorPos() { return clientCursorPos; }
	Vec2i GetCursorDeltaMove() { return mouseDelta; }

protected:
	struct KeyInfo
	{
		std::queue<bool> keyDownQueue;
		bool bStateDown = false;
	}; 

	// Storing key downs (current, previous) frame
	KeyInfo keyDown[(int)eKey::COUNT];

	// Mouse inputs
	bool bMouseDownCurFrame[(int)eMouseButton::COUNT];
	bool bMouseDownPrevFrame[(int)eMouseButton::COUNT];
	Vec2i cursorLastClickStartPos[(int)eMouseButton::COUNT];

	Vec2i mouseDelta;
	Vec2i clientCursorPos;

	// Keyboard delegates
	Delegate<void()> onKeyDown[(int)eKey::COUNT];
	Delegate<void()> onKeyPressed[(int)eKey::COUNT];
	Delegate<void()> onKeyReleased[(int)eKey::COUNT];

	// Mouse delegates
	Delegate<void(const Vec2i& clientCursorPos)> onMousePressed[(int)eMouseButton::COUNT];
	Delegate<void(const Vec2i& clientCursorPos)> onMouseReleased[(int)eMouseButton::COUNT];
	Delegate<void(const Vec2i& clientCursorPos)> onMouseDown[(int)eMouseButton::COUNT];
	Delegate<void(const Vec2i& mouseDelta, const Vec2i& clientCursorPos)> onMouseMove;
};

} // namespace inl::core