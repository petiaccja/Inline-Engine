#pragma once

#include "InputEvents.hpp"
#include "../Event.hpp"
#include <thread>
#include <atomic>
#include <map>
#include <set>
#include <mutex>
#include "../Singleton.hpp"


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


	/// <summary> Hints the size of the event queue for polling. </summary>
	void SetQueueSizeHint(size_t queueSize);

	/// <summary> Returns whether events were dropped due to not fitting into polling queue. </summary>
	bool GetEventDropped();

	/// <summary> Pop an event from the polling queue. </summary>
	/// <returns> True if there was an event to pop, false if queue is empty. </returns>
	bool PopEvent(InputEvent& out);


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
		static eKey TranslateKey(unsigned vkey);

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
			((*it)->*eventMember)(evt);
		}
	}
}



} // namespace inl