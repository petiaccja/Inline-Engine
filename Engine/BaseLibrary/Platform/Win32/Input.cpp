#include "Input.hpp"
#include "../../Exception/Exception.hpp"


namespace inl {


Input::Input() {
	m_deviceId = InvalidDeviceId;
	m_queueMode = eInputQueueMode::IMMEDIATE;
	m_queueSize = 10000;
}

Input::Input(size_t deviceId) {
	m_queueMode = eInputQueueMode::IMMEDIATE;
	m_queueSize = 10000;
	RawInputSource::GetInstance()->AddInput(this, deviceId);
}

Input::~Input() {
	RawInputSource::GetInstance()->RemoveInput(this);
}


void Input::SetQueueSizeHint(size_t queueSize) {
	m_queueSize = queueSize;
}


void Input::SetQueueMode(eInputQueueMode mode) {
	m_queueMode = mode;
}


bool Input::CallEvents() {
	std::unique_lock<decltype(m_queueMtx)> lk(m_queueMtx);

	bool eventDropped = m_eventDropped;
	m_eventDropped = false;
	std::queue<InputEvent> eventQueue = std::move(m_eventQueue);

	lk.unlock();

	while (!eventQueue.empty()) {
		InputEvent evt = eventQueue.front();
		eventQueue.pop();
		switch (evt.type) {
			case eInputEventType::MOUSE_BUTTON: 
				OnMouseButton(evt.mouseButton);
				break;
			case eInputEventType::MOUSE_MOVE:
				OnMouseMove(evt.mouseMove);
				break;
			case eInputEventType::KEYBOARD:
				OnKeyboard(evt.keyboard);
				break;
			case eInputEventType::JOYSTICK_BUTTON: 
				OnJoystickButton(evt.joystickButton);
				break;
			case eInputEventType::JOYSTICK_MOVE:
				OnJoystickMove(evt.joystickMove);
				break;
			default:;
		}
	}

	return eventDropped;
}


eInputQueueMode Input::GetQueueMode() const {
	return m_queueMode;
}


std::vector<InputDevice> Input::GetDeviceList() {
	UINT count = 0;
	GetRawInputDeviceList(nullptr, &count, sizeof(RAWINPUTDEVICELIST));
	std::vector<RAWINPUTDEVICELIST> list(count);
	GetRawInputDeviceList(list.data(), &count, sizeof(RAWINPUTDEVICELIST));

	std::vector<InputDevice> devices;
	for (auto& d : list) {
		InputDevice device;
		device.id = (size_t)d.hDevice;
		switch (d.dwType) {
			case RIM_TYPEKEYBOARD:
				device.type = eInputSourceType::KEYBOARD; break;
			case RIM_TYPEMOUSE:
				device.type = eInputSourceType::MOUSE; break;
			case RIM_TYPEHID:
				continue; // currently only keyboard and mouse
				device.type = eInputSourceType::JOYSTICK; break;
		}
		unsigned dataSize = 0;
		unsigned result;
		result = GetRawInputDeviceInfoA(d.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
		if (result == 0) {
			std::vector<char> deviceName(dataSize + 1, '\0');
			GetRawInputDeviceInfoA(d.hDevice, RIDI_DEVICENAME, deviceName.data(), &dataSize);
			device.name = std::string(deviceName.begin(), deviceName.end());
		}

		devices.push_back(device);
	}

	return devices;
}


Input::RawInputSourceBase::RawInputSourceBase() {
	m_messageLoopThread = std::thread([this] {
		MessageLoopThreadFunc();
	});
}

Input::RawInputSourceBase::~RawInputSourceBase() {
	SendMessage(m_messageLoopWindow, WM_CLOSE, 0, 0);
	if (m_messageLoopThread.joinable()) {
		m_messageLoopThread.join();
	}
}

void Input::RawInputSourceBase::AddInput(Input* input, size_t device) {
	std::lock_guard<std::mutex> lk(m_mtx);
	auto group = m_sources.find(device);
	if (group == m_sources.end()) {
		group = m_sources.insert({ device, {} }).first;
	}
	group->second.insert(input);
}

void Input::RawInputSourceBase::RemoveInput(Input* input, size_t device) {
	std::lock_guard<std::mutex> lk(m_mtx);
	if (device != InvalidDeviceId) {
		auto group = m_sources.find(device);
		if (group != m_sources.end()) {
			group->second.erase(input);
		}
	}
	else {
		for (auto& group : m_sources) {
			group.second.erase(input);
		}
	}
}


LRESULT __stdcall Input::RawInputSourceBase::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	RawInputSourceBase* instance = reinterpret_cast<RawInputSourceBase*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	switch (msg) {
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_NCCREATE:
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		case WM_INPUT: {
			UINT dwSize;

			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
			std::vector<BYTE> lpb(dwSize);

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				return 0;
			}

			RAWINPUT* rawInput = (RAWINPUT*)lpb.data();
			instance->ProcessInput(*rawInput);
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


void Input::RawInputSourceBase::ProcessInput(const RAWINPUT& rawInput) {
	std::unique_lock<std::mutex> lk(m_mtx);

	size_t deviceId = reinterpret_cast<size_t>(rawInput.header.hDevice);

	switch (rawInput.header.dwType) {
		case RIM_TYPEMOUSE: {
			RAWMOUSE mouse = rawInput.data.mouse;

			POINT point = { 0, 0 };
			GetCursorPos(&point);

			// Movement events.
			if ((mouse.usFlags & MOUSE_MOVE_RELATIVE)
				&& (mouse.lLastX != 0 || mouse.lLastY != 0))
			{
				MouseMoveEvent evt;
				evt.relx = mouse.lLastX;
				evt.rely = mouse.lLastY;
				evt.absx = point.x;
				evt.absy = point.y;

				CallEvents(deviceId, &Input::OnMouseMove, evt);
			}
			else {
				MouseMoveEvent evt;
				evt.relx = 0;
				evt.rely = 0;
				evt.absx = mouse.lLastX;
				evt.absy = mouse.lLastY;

				CallEvents(deviceId, &Input::OnMouseMove, evt);
			}

			// Button events.
			auto CallButtonEvent = [this, &point, deviceId](unsigned short flags, unsigned short checkFlag, eMouseButton btn, eKeyState state) {
				if (flags & checkFlag) {
					MouseButtonEvent evt;
					evt.button = btn;
					evt.state = state;
					evt.x = point.x;
					evt.y = point.y;
					CallEvents(deviceId, &Input::OnMouseButton, evt);
				}
			};
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_LEFT_BUTTON_DOWN, eMouseButton::LEFT, eKeyState::DOWN);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_LEFT_BUTTON_UP, eMouseButton::LEFT, eKeyState::UP);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_RIGHT_BUTTON_DOWN, eMouseButton::RIGHT, eKeyState::DOWN);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_RIGHT_BUTTON_UP, eMouseButton::RIGHT, eKeyState::UP);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_MIDDLE_BUTTON_DOWN, eMouseButton::MIDDLE, eKeyState::DOWN);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_MIDDLE_BUTTON_UP, eMouseButton::MIDDLE, eKeyState::UP);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_BUTTON_4_DOWN, eMouseButton::EXTRA1, eKeyState::DOWN);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_BUTTON_4_UP, eMouseButton::EXTRA1, eKeyState::UP);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_BUTTON_5_DOWN, eMouseButton::EXTRA2, eKeyState::DOWN);
			CallButtonEvent(mouse.usButtonFlags, RI_MOUSE_BUTTON_5_UP, eMouseButton::EXTRA2, eKeyState::UP);
			break;
		}
		case RIM_TYPEKEYBOARD: {
			KeyboardEvent evt;
			evt.key = impl::TranslateKey(rawInput.data.keyboard.VKey);
			evt.state = rawInput.data.keyboard.Flags & RI_KEY_BREAK ? eKeyState::UP : eKeyState::DOWN;

			CallEvents(deviceId, &Input::OnKeyboard, evt);
			break;
		}
		case RIM_TYPEHID: {
			break;
		}
	}
}


void Input::RawInputSourceBase::MessageLoopThreadFunc() {
	// Create a basic invisible window just for the message loop.
	WNDCLASSA wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = "INL_INPUT_CAPTURE";
	wc.lpfnWndProc = &WndProc;

	ATOM cres = RegisterClassA(&wc);
	if (cres == 0) {
		DWORD error = GetLastError();
		throw RuntimeException("Could not register window class for raw input.", std::to_string(error));
	}

	HWND hwnd = CreateWindowA("INL_INPUT_CAPTURE",
		"Inl input capture",
		WS_OVERLAPPEDWINDOW,
		0, 0, 10, 10,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		this);

	if (!hwnd) {
		DWORD error = GetLastError();
		throw RuntimeException("Could not create dummy window for raw input.", std::to_string(error));
	}

	m_messageLoopWindow = hwnd;
	UpdateWindow(hwnd);

	// Register for raw input.
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = m_messageLoopWindow;

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = m_messageLoopWindow;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {
		throw RuntimeException("Could not register for raw input.");
	}

	// Start processing raw input messages.
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


namespace impl {
eKey TranslateKey(unsigned vkey) {
	switch (vkey) {
		case VK_BACK: return eKey::BACKSPACE;
		case VK_TAB: return eKey::TAB;
		case VK_CLEAR: return eKey::CLEAR;
		case VK_RETURN: return eKey::ENTER;
		case VK_SHIFT: return eKey::SHIFT_LEFT;
		case VK_CONTROL: return eKey::CONTROL_LEFT;
		case VK_MENU: return eKey::ALT_LEFT; // alt
		case VK_PAUSE: return eKey::PAUSE;
		case VK_CAPITAL: return eKey::CAPS_LOCK;  // caps lock
		case VK_ESCAPE: return eKey::ESCAPE;
		case VK_SPACE: return eKey::SPACE;
		case VK_PRIOR: return eKey::PAGE_UP; // page up
		case VK_NEXT: return eKey::PAGE_DOWN; // page down
		case VK_END: return eKey::END;
		case VK_HOME: return eKey::HOME;
		case VK_LEFT: return eKey::LEFT;
		case VK_UP: return eKey::UP;
		case VK_RIGHT: return eKey::RIGHT;
		case VK_DOWN: return eKey::DOWN;
		case VK_SELECT: return eKey::SELECT;
		case VK_PRINT: return eKey::PRINT;
		case VK_SNAPSHOT: return eKey::PRINT_SCREEN; // print screen
		case VK_INSERT: return eKey::INSERT;
		case VK_DELETE: return eKey::DELETE;
		case VK_LWIN: return eKey::WINKEY_LEFT;
		case VK_RWIN: return eKey::WINKEY_RIGHT;
		case VK_SLEEP: return eKey::SLEEP;

			// numpad
		case VK_MULTIPLY: return eKey::MULTIPLY;
		case VK_ADD: return eKey::ADD;
		case VK_SEPARATOR: return eKey::SPEPARATOR;
		case VK_SUBTRACT: return eKey::SUBTRACT;
		case VK_DECIMAL: return eKey::DECIMAL;
		case VK_DIVIDE: return eKey::DIVIDE;

			// misc
		case VK_NUMLOCK: return eKey::NUM_LOCK;
		case VK_SCROLL: return eKey::SCROLL_LOCK; // scroll lock

		// modifiers
		case VK_LSHIFT: return eKey::SHIFT_LEFT;
		case VK_RSHIFT: return eKey::SHIFT_RIGHT;
		case VK_LCONTROL: return eKey::CONTROL_LEFT;
		case VK_RCONTROL: return eKey::CONTROL_RIGHT;
		case VK_LMENU: return eKey::ALT_LEFT; // alt
		case VK_RMENU: return eKey::ALT_RIGHT; // alt gr

		// browser
		case VK_BROWSER_BACK: return eKey::UNKNOWN;
		case VK_BROWSER_FORWARD: return eKey::UNKNOWN;
		case VK_BROWSER_REFRESH: return eKey::UNKNOWN;
		case VK_BROWSER_STOP: return eKey::UNKNOWN;
		case VK_BROWSER_SEARCH: return eKey::UNKNOWN;
		case VK_BROWSER_FAVORITES: return eKey::UNKNOWN;
		case VK_BROWSER_HOME: return eKey::UNKNOWN;
		default: break;
	}

	// 0x30-0x39: 0-9 keys
	// 0x41-0x5A: A-Z keys
	// 0x60-0x69: numpad 0-9 keys
	// 0x70-0x87: F1-F24 keys
	if (0x30 <= vkey && vkey <= 0x39) {
		return static_cast<eKey>(static_cast<unsigned>(eKey::NUMBER_0) + vkey - 0x30);
	}
	else if (0x41 <= vkey && vkey <= 0x5A) {
		return static_cast<eKey>(static_cast<unsigned>(eKey::A) + vkey - 0x41);
	}
	else if (0x60 <= vkey && vkey <= 0x69) {
		return static_cast<eKey>(static_cast<unsigned>(eKey::NUMPAD_0) + vkey - 0x60);
	}
	else if (0x70 <= vkey && vkey <= 0x87) {
		return static_cast<eKey>(static_cast<unsigned>(eKey::F1) + vkey - 0x70);
	}

	return eKey::UNKNOWN;
}
}



} // namespace inl