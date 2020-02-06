#pragma once

#include "../../Event.hpp"
#include "../../Singleton.hpp"
#include "InputEvents.hpp"

#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <thread>


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef DELETE
#include <hidsdi.h>


namespace inl {

class Input;

enum class eInputSourceType {
	MOUSE,
	KEYBOARD,
	JOYSTICK,
};

enum class eInputQueueMode {
	IMMEDIATE,
	QUEUED,
};

struct InputDevice {
	eInputSourceType type;
	size_t id;
	std::string name;
};

struct InputSharedState {
	const size_t deviceId;
	volatile eInputQueueMode queueMode;
	volatile size_t queueSize;
	volatile bool eventDropped;
	Input* volatile parent; // Only for immediate mode.
	std::queue<InputEvent> eventQueue;
	std::unique_ptr<std::mutex> queueMtx = std::make_unique<std::mutex>();
};


class Input {
public:
	Input() = default;
	Input(Input&& rhs);
	Input& operator=(Input&& rhs);
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	Input(size_t deviceId);
	~Input();
	
	/// <summary> Tells how many events should be queued at maximum. Only applies to queued mode. </summary>
	void SetQueueSizeHint(size_t queueSize);

	/// <summary> Sets event calling mode: in immediate mode, events are called
	///		asynchonously from another thread; in queued mode, events are stored
	///		and you have to call <see cref="CallEvents"> manually to call all queued
	///		events on the caller's thread. </summary>
	void SetQueueMode(eInputQueueMode mode);

	/// <summary> Returns the currently set queueing mode. </summary>
	eInputQueueMode GetQueueMode() const;

	/// <summary> Calls all queued events synchronously on the caller's thread. </summary>
	/// <returns> False if some events were dropped due to too small queue size. </returns>
	bool CallEvents();

	/// <summary> Returns the list of available input devices that you can listen to. </summary>
	static std::vector<InputDevice> GetDeviceList();

	/// <summary> Returns the list of available input devices of specific type that you can listen to. </summary>
	static std::vector<InputDevice> GetDeviceList(eInputSourceType filter);

public:
	Event<MouseButtonEvent> OnMouseButton;
	Event<MouseMoveEvent> OnMouseMove;
	Event<KeyboardEvent> OnKeyboard;
	Event<JoystickButtonEvent> OnJoystickButton;
	Event<JoystickMoveEvent> OnJoystickMove;

private:
	std::shared_ptr<InputSharedState> m_sharedState;
};


class RawInputSourceBase {
	struct JoyState {
		std::vector<uint8_t> preparsedBuffer;
		std::vector<bool> buttonStates;
		std::vector<float> valueStates;
		std::vector<HIDP_BUTTON_CAPS> buttonCaps;
		std::vector<HIDP_VALUE_CAPS> valueCaps;
	};

public:
	RawInputSourceBase();
	~RawInputSourceBase();

	void AddInput(std::shared_ptr<InputSharedState> state);
	void RemoveInput(const std::shared_ptr<InputSharedState>& state);
	DWORD DbgTID() { return GetThreadId(m_messageLoopThread.native_handle()); }

private:
	static LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void ProcessInput(const RAWINPUT& rawInput);
	void MessageLoopThreadFunc();

	template <class EventArg>
	void CallEvents(size_t device, Event<EventArg> Input::*eventMember, const EventArg& evt) const;

	static JoyState GetJoyInfo(size_t deviceId);

private:
	std::mutex m_mtx;
	std::thread m_messageLoopThread;
	HWND m_messageLoopWindow = nullptr;
	std::map<size_t, std::set<std::shared_ptr<InputSharedState>>> m_sources;

	std::unordered_map<size_t, JoyState> m_joyStates; // deviceId -> joyState
};
using RawInputSource = Singleton<RawInputSourceBase>;



template <class EventArg>
void RawInputSourceBase::CallEvents(size_t device, Event<EventArg> Input::*eventMember, const EventArg& evt) const {
	auto group = m_sources.find(device);
	if (group != m_sources.end()) {
		for (auto it = group->second.begin(); it != group->second.end(); ++it) {
			InputSharedState& state = **it;
			std::lock_guard lkg(*state.queueMtx);
			eInputQueueMode queueMode = state.parent->GetQueueMode();
			if (queueMode == eInputQueueMode::IMMEDIATE) {
				(state.parent->*eventMember)(evt);
			}
			else {
				size_t queueSize = state.queueSize;
				state.eventQueue.push(InputEvent(evt));
				if (state.eventQueue.size() > queueSize) {
					state.eventDropped = true;
				}
				while (state.eventQueue.size() > queueSize) {
					state.eventQueue.pop();
				}
			}
		}
	}
}

namespace impl {
	eKey TranslateKey(unsigned vkey);
}


} // namespace inl