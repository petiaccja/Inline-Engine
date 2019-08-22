#include "WindowEventSystem.hpp"


void WindowEventSystem::Update(float elapsed) {
	for (auto window : m_windows) {
		window->CallEvents();
	}
}


void WindowEventSystem::SetWindows(std::vector<inl::Window*> windows) {
	m_windows = std::move(windows);
}
