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

	void SimulateMouseBtnPress(eMouseBtn button);
	void SimulateMouseBtnRelease(eMouseBtn button);
	void SimulateMouseMove(const Vec2i& mouseDelta, const Vec2i& clientCursorPos);

	void BindKeyDown(eKey key, const std::function<void()>& function);
	void BindKeyPress(eKey key, const std::function<void()>& function);
	void BindKeyRelease(eKey key, const std::function<void()>& function);

	void BindMousePress(eMouseBtn button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseRelease(eMouseBtn button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseDown(eMouseBtn button, const std::function<void(const Vec2i& clientCursorPos)>& function);
	void BindMouseMove(const std::function<void(const Vec2i& mouseDelta, const Vec2i& clientCursorPos)>& function);

	void ClearFrameData();
	void Update();

	bool IsKeyDown(eKey key);
	bool IsKeyPressed(eKey key);
	bool IsKeyReleased(eKey key);

	bool IsMousePressed(eMouseBtn eButton);
	bool IsMouseReleased(eMouseBtn eButton);
	bool IsMouseDown(eMouseBtn eButton);
	bool IsMouseClicked(eMouseBtn eButton);

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
	bool bMouseDownCurFrame[(int)eMouseBtn::COUNT];
	bool bMouseDownPrevFrame[(int)eMouseBtn::COUNT];
	Vec2i cursorLastClickStartPos[(int)eMouseBtn::COUNT];

	Vec2i mouseDelta;
	Vec2i clientCursorPos;

	// Keyboard delegates
	Delegate<void()> onKeyDown[(int)eKey::COUNT];
	Delegate<void()> onKeyPressed[(int)eKey::COUNT];
	Delegate<void()> onKeyReleased[(int)eKey::COUNT];

	// Mouse delegates
	Delegate<void(const Vec2i& clientCursorPos)> onMousePressed[(int)eMouseBtn::COUNT];
	Delegate<void(const Vec2i& clientCursorPos)> onMouseReleased[(int)eMouseBtn::COUNT];
	Delegate<void(const Vec2i& clientCursorPos)> onMouseDown[(int)eMouseBtn::COUNT];
	Delegate<void(const Vec2i& mouseDelta, const Vec2i& clientCursorPos)> onMouseMove;
};

} // namespace inl::core