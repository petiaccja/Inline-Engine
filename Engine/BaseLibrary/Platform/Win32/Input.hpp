#pragma once

#include "InputEvents.hpp"
#include "../../Event.hpp"
#include <thread>
#include <atomic>
#include <map>
#include <set>
#include <mutex>
#include <queue>
#include "../../Singleton.hpp"


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef DELETE


namespace inl {


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



class Input {
	friend class RawInputSourceBase;
public:
	Input();
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

public:
	Event<MouseButtonEvent> OnMouseButton;
	Event<MouseMoveEvent> OnMouseMove;
	Event<KeyboardEvent> OnKeyboard;
	Event<JoystickButtonEvent> OnJoystickButton;
	Event<JoystickMoveEvent> OnJoystickMove;

private:
	static constexpr size_t InvalidDeviceId = -1;
	size_t m_deviceId;

	volatile eInputQueueMode m_queueMode;
	volatile size_t m_queueSize;
	volatile bool m_eventDropped;
	std::queue<InputEvent> m_eventQueue;
	std::mutex m_queueMtx;

private:
	class RawInputSourceBase {
		static constexpr size_t InvalidDeviceId = -1;
	public:
		RawInputSourceBase();
		~RawInputSourceBase();

		void AddInput(Input* input, size_t device);
		void RemoveInput(Input* input, size_t device = InvalidDeviceId);
	private:
		static LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void ProcessInput(const RAWINPUT& rawInput);
		void MessageLoopThreadFunc();

		template <class EventArg>
		void CallEvents(size_t device, Event<EventArg> Input::*eventMember, const EventArg& evt) const;

		std::mutex m_mtx;
		std::thread m_messageLoopThread;
		HWND m_messageLoopWindow = 0;
		std::map<size_t, std::set<Input*>> m_sources;
	};
	using RawInputSource = Singleton<RawInputSourceBase>;
};



template <class EventArg>
void Input::RawInputSourceBase::CallEvents(size_t device, Event<EventArg> Input::*eventMember, const EventArg& evt) const {
	auto group = m_sources.find(device);
	if (group != m_sources.end()) {
		for (auto it = group->second.begin(); it != group->second.end(); ++it) {
			Input& input = **it;
			eInputQueueMode queueMode = input.GetQueueMode();
			if (queueMode == eInputQueueMode::IMMEDIATE) {
				(input.*eventMember)(evt);
			}
			else {
				std::lock_guard<decltype(Input::m_queueMtx)> lkg(input.m_queueMtx);
				size_t queueSize = input.m_queueSize;
				input.m_eventQueue.push(InputEvent(evt));
				if (input.m_eventQueue.size() > queueSize) {
					input.m_eventDropped = true;
				}
				while (input.m_eventQueue.size() > queueSize) {
					input.m_eventQueue.pop();
				}
			}
		}
	}
}


namespace impl {
eKey TranslateKey(unsigned vkey);
}



} // namespace inl