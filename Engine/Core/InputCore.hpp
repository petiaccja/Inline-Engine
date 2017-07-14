#pragma once
#include <BaseLibrary/Platform/Sys.hpp>
#include <array>
#include <functional>
#include <queue>

namespace inl::core {

class InputCore
{
public:
	InputCore();

public:
	void KeyPress(eKey key);
	void KeyRelease(eKey key);

	void MouseRightPress();
	void MouseRightRelease();
	void MouseLeftPress();
	void MouseLeftRelease();
	void MouseMidPress();
	void MouseMidRelease();
	
	void MouseMove(const Vec2i& mouseDelta, const Vec2i& clientMousePos);

	void RegOnKeyDown(eKey key, const std::function<void()> callb);
	void RegOnKeyPressed(eKey key, const std::function<void()> callb);
	void RegOnKeyReleased(eKey key, const std::function<void()> callb);

	void RegOnMouseRightPressed(  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseRightReleased( const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseRightDown(	  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseLeftPressed(	  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseLeftReleased(  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseLeftDown(	  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseMidPressed(	  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseMidReleased(	  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseMidDown(		  const std::function<void(const Vec2i& clientMousePos)> callb);
	void RegOnMouseMove(		  const std::function<void(const Vec2i& mouseDelta, const Vec2i& clientMousePos)> callb);

	void ClearFrameData();
	void Update();

	bool IsKeyDown(eKey key);
	bool IsKeyPressed(eKey key);
	bool IsKeyReleased(eKey key);

	bool IsRightMouseBtnPressed();
	bool IsRightMouseBtnReleased();
	bool IsRightMouseBtnDown();
	bool IsLeftMouseBtnPressed();
	bool IsLeftMouseBtnReleased();
	bool IsLeftMouseBtnDown();
	bool IsMidMouseBtnPressed();
	bool IsMidMouseBtnReleased();
	bool IsMidMouseBtnDown();
	
	bool IsMouseMove(Vec2i& mouseDelta_out);

protected:
	struct KeyInfo
	{
		std::queue<bool> keyDownQueue;
		bool bStateDown;
	};

	// Storing key downs (current, previous) frame
	std::array<KeyInfo, (size_t)COUNT_eKey> keyDownArray;

	// Mouse inputs
	bool bMouseRightDownCurFrame;
	bool bMouseRightDownPrevFrame;
	bool bMouseLeftDownCurFrame;
	bool bMouseLeftDownPrevFrame;
	bool bMouseMidDownCurFrame;
	bool bMouseMidDownPrevFrame;
	Vec2i mouseDelta;
	Vec2i clientMousePos;

	// Registered keyboard callbacks
	std::vector<std::pair<eKey, std::function<void()>>> onKeyDownCallbacks;
	std::vector<std::pair<eKey, std::function<void()>>> onKeyPressedCallbacks;
	std::vector<std::pair<eKey, std::function<void()>>> onKeyReleasedCallbacks;

	// Registered mouse callbacks
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseRightPressedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseRightReleasedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseRightDownCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseLeftPressedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseLeftReleasedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseLeftDownCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseMidPressedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseMidReleasedCallbacks;
	std::vector<std::function<void(const Vec2i& clientMousePos)>> onMouseMidDownCallbacks;
	std::vector<std::function<void(const Vec2i& mouseDelta, const Vec2i& clientMousePos)>> onMouseMoveCallbacks;
};

} // namespace inl::core