#include "UserInputSystem.hpp"


UserInputSystem::UserInputSystem(std::shared_ptr<ActionHeap> actionHeap) : m_actionHeap(actionHeap) {
	RegisterDevices();
	RegisterEvents();
}


UserInputSystem::UserInputSystem(UserInputSystem&& rhs)
	: m_actionHeap(rhs.m_actionHeap),
	  m_sources(std::move(rhs.m_sources)) {
	RegisterDevices();
	RegisterEvents();
}


UserInputSystem& UserInputSystem::operator=(UserInputSystem&& rhs) {
	m_actionHeap = rhs.m_actionHeap;
	m_sources = std::move(rhs.m_sources);
	return *this;
}


void UserInputSystem::Update(float elapsed) {
	m_keyBoardInput.CallEvents();
	m_mouseInput.CallEvents();
	if (IsEnabled()) {
		for (auto& [_ignore, source] : m_sources) {
			source->Update(*m_actionHeap);
		}
	}
}

inline void UserInputSystem::Remove(const UserInputSource& source) {
	for (auto it = m_sources.begin(); it != m_sources.end(); ++it) {
		if (it->second.get() == &source) {
			m_sources.erase(it);
			return;
		}
	}
	throw inl::KeyNotFoundException();
}

inline void UserInputSystem::Clear() {
	m_sources.clear();
}

inline size_t UserInputSystem::Size() const {
	return m_sources.size();
}


void UserInputSystem::SetEnabled(bool enabled) { m_enabled = enabled; }

void UserInputSystem::Enable() { m_enabled = true; }

void UserInputSystem::Disable() { m_enabled = false; }

bool UserInputSystem::IsEnabled() const { return m_enabled; }


void UserInputSystem::OnKeyboard(inl::KeyboardEvent evt) {
	if (IsEnabled()) {
		for (auto& [_ignore, source] : m_sources) {
			source->OnKeyboard(evt, *m_actionHeap);
		}
	}
}

void UserInputSystem::OnMouseButton(inl::MouseButtonEvent evt) {
	if (IsEnabled()) {
		for (auto& [_ignore, source] : m_sources) {
			source->OnMouseButton(evt, *m_actionHeap);
		}
	}
}

void UserInputSystem::OnMouseMove(inl::MouseMoveEvent evt) {
	if (IsEnabled()) {
		for (auto& [_ignore, source] : m_sources) {
			source->OnMouseMove(evt, *m_actionHeap);
		}
	}
}

void UserInputSystem::OnMouseWheel(inl::MouseWheelEvent evt) {
	if (IsEnabled()) {
		for (auto& [_ignore, source] : m_sources) {
			source->OnMouseWheel(evt, *m_actionHeap);
		}
	}
}

void UserInputSystem::RegisterDevices() {
	const auto&& devices = inl::Input::GetDeviceList();
	for (auto& device : devices) {
		if (device.type == inl::eInputSourceType::KEYBOARD) {
			m_keyBoardInput = inl::Input{ device.id };
			break;
		}
	}
	for (auto& device : devices) {
		if (device.type == inl::eInputSourceType::MOUSE) {
			m_mouseInput = inl::Input{ device.id };
			break;
		}
	}
}

void UserInputSystem::RegisterEvents() {
	m_keyBoardInput.OnKeyboard += inl::Delegate<void(inl::KeyboardEvent)>{ &UserInputSystem::OnKeyboard, this };
	m_mouseInput.OnMouseButton += inl::Delegate<void(inl::MouseButtonEvent)>{ &UserInputSystem::OnMouseButton, this };
	m_mouseInput.OnMouseMove += inl::Delegate<void(inl::MouseMoveEvent)>{ &UserInputSystem::OnMouseMove, this };
}
