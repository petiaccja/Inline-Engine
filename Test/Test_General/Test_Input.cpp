#include "Test.hpp"
#include <csignal>
#include <iostream>
#include <BaseLibrary/Platform/Input.hpp>


using namespace std;
using namespace inl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestInput : public AutoRegisterTest<TestInput> {
public:
	TestInput() {}

	static std::string Name() {
		return "Input";
	}
	int Run() override;
	void OnClick(MouseButtonEvent evt);
	void OnKey(KeyboardEvent evt);
	void OnAxis(JoystickMoveEvent evt);
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


static volatile bool run = true;

void SignalHandler(int) {
	run = false;
}

int TestInput::Run() {
	auto devices = Input::GetDeviceList();

	size_t mouseId = -1;
	size_t keyboardId = -1;
	size_t joystickId = -1;
	cout << "Input devices:" << endl;
	for (int i = 0; i < devices.size(); ++i) {
		if (devices[i].type == eInputSourceType::MOUSE) {
			mouseId = devices[i].id;
		}
		if (devices[i].type == eInputSourceType::KEYBOARD) {
			keyboardId = devices[i].id;
		}
		if (devices[i].type == eInputSourceType::JOYSTICK) {
			joystickId = devices[i].id;
		}
		cout << "id = " << devices[i].id;
		cout << ", name = " << devices[i].name;
		switch (devices[i].type) {
			case eInputSourceType::KEYBOARD: cout << ", keyboard" << endl; break;
			case eInputSourceType::MOUSE: cout << ", mouse" << endl; break;
			case eInputSourceType::JOYSTICK: cout << ", joystick" << endl; break;
		}
	}

	std::unique_ptr<Input> inputMouse, inputKeyboard, inputJoystick;
	if (mouseId != -1) {
		inputMouse = std::make_unique<Input>(mouseId);
		inputMouse->SetQueueMode(eInputQueueMode::QUEUED);

		inputMouse->OnMouseButton += Delegate<void(MouseButtonEvent)>{ &TestInput::OnClick, this };
	}
	if (keyboardId != -1) {
		inputKeyboard = std::make_unique<Input>(keyboardId);

		inputKeyboard->OnKeyboard += Delegate<void(KeyboardEvent)>{ &TestInput::OnKey, this };
	}
	if (joystickId != -1) {
		inputJoystick = std::make_unique<Input>(joystickId);

		inputJoystick->OnJoystickMove += Delegate<void(JoystickMoveEvent)>{ &TestInput::OnAxis, this };
	}
	
	

	cout << "Press Control-C to quit." << endl;
	signal(SIGINT, SignalHandler);

	while (run) {
		this_thread::sleep_for(chrono::milliseconds(50));
		inputMouse->CallEvents();
	}


	return 0;
}


void TestInput::OnClick(MouseButtonEvent evt) {
	std::string btn;
	switch (evt.button) {
		case eMouseButton::LEFT: btn = "left"; break;
		case eMouseButton::RIGHT: btn = "right"; break;
		case eMouseButton::MIDDLE: btn = "middle"; break;
		case eMouseButton::EXTRA1: btn = "e1"; break;
		case eMouseButton::EXTRA2: btn = "e2"; break;
	}
	cout << "click: " << evt.x << ", " << evt.y << " " << btn << (evt.state == eKeyState::DOWN ? " down" : " up") << endl;
}


void TestInput::OnKey(KeyboardEvent evt) {
	char btn = '?';
	if (eKey::A <= evt.key && evt.key <= eKey::Z) {
		btn = (char)evt.key - (char)eKey::A + 'a';
	}
	if (eKey::NUMBER_0 <= evt.key && evt.key <= eKey::NUMBER_9) {
		btn = (char)evt.key - (char)eKey::NUMBER_0 + '0';
	}
	cout << btn << endl;
}

void TestInput::OnAxis(JoystickMoveEvent evt) {
	cout << "Axis " << evt.axis << " = " << evt.absPos << endl;
}