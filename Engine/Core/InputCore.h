#pragma once
#include <BaseLibrary/Platform/Sys.hpp>
#include <array>
#include <functional>
#include <queue>

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
	
	void MouseMove(const ivec2& mouseDelta, const ivec2& clientMousePos);

	void RegOnKeyDown(eKey key, const std::function<void()> callb);
	void RegOnKeyPressed(eKey key, const std::function<void()> callb);
	void RegOnKeyReleased(eKey key, const std::function<void()> callb);

	void RegOnMouseRightPressed(  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseRightReleased( const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseRightDown(	  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseLeftPressed(	  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseLeftReleased(  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseLeftDown(	  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseMidPressed(	  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseMidReleased(	  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseMidDown(		  const std::function<void(const ivec2& clientMousePos)> callb);
	void RegOnMouseMove(		  const std::function<void(const ivec2& mouseDelta, const ivec2& clientMousePos)> callb);

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
	
	bool IsMouseMove(ivec2& mouseDelta_out);

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
	ivec2 mouseDelta;
	ivec2 clientMousePos;

	// Registered keyboard callbacks
	std::vector<std::pair<eKey, std::function<void()>>> onKeyDownCallbacks;
	std::vector<std::pair<eKey, std::function<void()>>> onKeyPressedCallbacks;
	std::vector<std::pair<eKey, std::function<void()>>> onKeyReleasedCallbacks;

	// Registered mouse callbacks
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseRightPressedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseRightReleasedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseRightDownCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseLeftPressedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseLeftReleasedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseLeftDownCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseMidPressedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseMidReleasedCallbacks;
	std::vector<std::function<void(const ivec2& clientMousePos)>> onMouseMidDownCallbacks;
	std::vector<std::function<void(const ivec2& mouseDelta, const ivec2& clientMousePos)>> onMouseMoveCallbacks;
};