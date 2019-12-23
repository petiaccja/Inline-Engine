#pragma once

#include <any>
#include <memory>
#include <queue>
#include <unordered_set>


// TODO: Please do this properly...

namespace inl {

class ActionSystem;


class BasicActionListener {
public:
	virtual ~BasicActionListener() = default;
	virtual void TryEvent(const std::any& event) = 0;
	void SetEventSystem(ActionSystem* system);

protected:
	template <class Event>
	void Send(Event&& event);

private:
	ActionSystem* m_system = nullptr;
};


template <class Listener, class... AcceptedEvents>
class ActionListener : public BasicActionListener {
public:
	void TryEvent(const std::any& event) override;
};


class ActionSystem {
public:
	template <class Event>
	void Send(Event&& event);

	void Update();

	void Add(std::shared_ptr<BasicActionListener> listener);
	void Remove(const std::shared_ptr<BasicActionListener>& listener);
	void Clear();
	
private:
	std::queue<std::any> m_events;
	std::unordered_set<std::shared_ptr<BasicActionListener>> m_listeners;
};



inline void BasicActionListener::SetEventSystem(ActionSystem* system) {
	m_system = system;
}

inline void ActionSystem::Update() {
	auto eventsCopy = std::move(m_events);
	while (!eventsCopy.empty()) {
		std::any event = std::move(eventsCopy.front());
		for (auto& listener : m_listeners) {
			listener->TryEvent(event);
		}
		eventsCopy.pop();
	}
}

inline void ActionSystem::Add(std::shared_ptr<BasicActionListener> listener) {
	m_listeners.insert(listener);
	listener->SetEventSystem(this);
}

inline void ActionSystem::Remove(const std::shared_ptr<BasicActionListener>& listener) {
	listener->SetEventSystem(nullptr);
	m_listeners.erase(listener);
}

inline void ActionSystem::Clear() {
	m_listeners.clear();
	
}

template <class Event>
void BasicActionListener::Send(Event&& event) {
	if (m_system) {
		m_system->Send(std::forward<Event>(event));
	}
}

template <class Listener, class... AcceptedEvents>
void ActionListener<Listener, AcceptedEvents...>::TryEvent(const std::any& event) {
	constexpr bool invocableForAll = (true && ... && std::is_invocable_v<Listener, AcceptedEvents>);
	static_assert(invocableForAll, "Make sure your Listener type can be invoked with all types listed for the ActionListener.");

	const auto& type = event.type();

	auto CallIfMatch = [this, &type, &event](auto* dummy) {
		using TrialType = std::decay_t<decltype(*dummy)>;
		if (type == typeid(TrialType)) {
			static_cast<Listener&> (*this)(std::any_cast<const TrialType&>(event));
		}
	};

	(..., CallIfMatch((AcceptedEvents*)nullptr));
}

template <class Event>
void ActionSystem::Send(Event&& event) {
	m_events.push(std::any{ std::forward<Event>(event) });
}


} // namespace inl