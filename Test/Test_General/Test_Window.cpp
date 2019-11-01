#include "Test.hpp"
#include <csignal>
#include <iostream>
#include <BaseLibrary/Platform/Window.hpp>


using namespace std;
using namespace inl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestWindow : public AutoRegisterTest<TestWindow> {
public:
	TestWindow() {}

	static std::string Name() {
		return "Window";
	}
	int Run() override;
	void OnClick(MouseButtonEvent evt);
	void OnKey(KeyboardEvent evt);
	void OnChar(char32_t evt);
	void OnClose() {
		closed = true;
	}
	void OnDropped(DragDropEvent evt);
private:
	static int a;
	volatile bool closed = false;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestWindow::Run() {
	try {
		Window window{ (const char*)u8"Test Window", Vec2u{640, 480}, false, true };
		//window.SetQueueMode(eInputQueueMode::QUEUED);

		window.OnMouseButton += Delegate<void(MouseButtonEvent)>{&TestWindow::OnClick, this};
		//window.OnKeyboard += {&TestWindow::OnKey, this};
		window.OnCharacter += Delegate<void(char32_t)>{&TestWindow::OnChar, this};
		window.OnClose += Delegate<void()>{&TestWindow::OnClose, this};
		window.OnDrop += Delegate<void(DragDropEvent)>{&TestWindow::OnDropped, this};

		while (!closed) {
			this_thread::sleep_for(chrono::milliseconds(50));
			window.CallEvents();
		}
		cout << "\nWindow closed." << endl;
	}
	catch (std::exception& ex) {
		cout << ex.what() << endl;
	}

	return 0;
}


void TestWindow::OnClick(MouseButtonEvent evt) {
	std::string btn;
	switch (evt.button) {
		case eMouseButton::LEFT: btn = "left"; break;
		case eMouseButton::RIGHT: btn = "right"; break;
		case eMouseButton::MIDDLE: btn = "middle"; break;
		case eMouseButton::EXTRA1: btn = "e1"; break;
		case eMouseButton::EXTRA2: btn = "e2"; break;
	}
	cout << "click: " << evt.x << ", " << evt.y << " " << btn 
		<< (evt.state == eKeyState::DOWN ? " down" : (evt.state == eKeyState::UP ? " up" : " double")) << endl;
}


void TestWindow::OnKey(KeyboardEvent evt) {
	char btn = '?';
	if (eKey::A <= evt.key && evt.key <= eKey::Z) {
		btn = (char)evt.key - (char)eKey::A + 'a';
	}
	if (eKey::NUMBER_0 <= evt.key && evt.key <= eKey::NUMBER_9) {
		btn = (char)evt.key - (char)eKey::NUMBER_0 + '0';
	}
	cout << btn << endl;
}


void TestWindow::OnChar(char32_t evt) {
	cout << (char)evt;
}


void TestWindow::OnDropped(DragDropEvent evt) {
	for (auto& f : evt.filePaths) {
		cout << f << endl;
	}
}