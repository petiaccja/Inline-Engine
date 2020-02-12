#pragma once

#include "ActionHeap.hpp"
#include "ActionSystem.hpp"
#include "UserInputSource.hpp"

#include <BaseLibrary/Platform/Input.hpp>
#include <GameLogic/System.hpp>

#include <map>
#include <optional>


class UserInputSystem : public inl::game::System<UserInputSystem>, public ActionSystem {
public:
	UserInputSystem();
	UserInputSystem(UserInputSystem&& rhs);
	UserInputSystem& operator=(UserInputSystem&& rhs);
	UserInputSystem(const UserInputSystem&) = delete;
	UserInputSystem& operator=(const UserInputSystem&) = delete;

	void ReactActions(ActionHeap& actions) override;
	void Update(float elapsed) override {}
	void EmitActions(ActionHeap& actions) override;

	template <class SourceT>
	void Insert(SourceT&& source) requires std::is_base_of_v<UserInputSource, SourceT>;
	template <class SourceT>
	void Remove() requires std::is_base_of_v<UserInputSource, SourceT>;
	void Remove(const UserInputSource& source);
	void Clear();
	size_t Size() const;

	template <class SourceT>
	SourceT& Get();
	template <class SourceT>
	const SourceT& Get() const;

	void SetEnabled(bool enabled);
	void Enable();
	void Disable();
	bool IsEnabled() const;

private:
	void OnKeyboard(inl::KeyboardEvent evt);
	void OnMouseButton(inl::MouseButtonEvent evt);
	void OnMouseMove(inl::MouseMoveEvent evt);
	void OnMouseWheel(inl::MouseWheelEvent evt);

	void RegisterDevices();
	void RegisterEvents();

private:
	inl::Input m_keyBoardInput;
	inl::Input m_mouseInput;
	bool m_enabled = true;
	std::map<std::type_index, std::unique_ptr<UserInputSource>> m_sources;
	std::optional<std::reference_wrapper<ActionHeap>> m_transientActionHeap;
};



template <class SourceT>
void UserInputSystem::Insert(SourceT&& source) requires std::is_base_of_v<UserInputSource, SourceT> {
	auto& v = m_sources[typeid(SourceT)];
	if (v) {
		throw inl::InvalidArgumentException("A specific type of source can only occur once.");
	}
	v = std::make_unique<SourceT>(std::forward<SourceT>(source));
}


template <class SourceT>
void UserInputSystem::Remove() requires std::is_base_of_v<UserInputSource, SourceT> {
	auto it = m_sources.find(typeid(SourceT));
	if (it != m_sources.end()) {
		m_sources.erase(it);
	}
	throw inl::KeyNotFoundException();
}


template <class SourceT>
SourceT& UserInputSystem::Get() {
	auto it = m_sources.find(typeid(SourceT));
	if (it != m_sources.end()) {
		return *it->second;
	}
	throw inl::KeyNotFoundException();
}


template <class SourceT>
const SourceT& UserInputSystem::Get() const {
	auto it = m_sources.find(typeid(SourceT));
	if (it != m_sources.end()) {
		return *it->second;
	}
	throw inl::KeyNotFoundException();
}
