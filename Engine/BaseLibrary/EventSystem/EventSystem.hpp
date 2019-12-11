#pragma once

#include <any>
#include <memory>
#include <queue>
#include <unordered_set>


// TODO: Please do this properly...

namespace inl {

class EventSystem;


class BasicEventListener {
public:
	virtual ~BasicEventListener() = default;
	virtual void TryEvent(const std::any& event) = 0;
	void SetEventSystem(EventSystem* system);

protected:
	template <class Event>
	void Send(Event&& event);

private:
	EventSystem* m_system = nullptr;
};


template <class Listener, class... AcceptedEvents>
class EventListener : public BasicEventListener {
public:
	void TryEvent(const std::any& event) override;
};


class EventSystem {
public:
	template <class Event>
	void Send(Event&& event);

	void Update();

	void Add(std::shared_ptr<BasicEventListener> listener);
	void Remove(const std::shared_ptr<BasicEventListener>& listener);
	void Clear();
	
private:
	std::queue<std::any> m_events;
	std::unordered_set<std::shared_ptr<BasicEventListener>> m_listeners;
};



inline void BasicEventListener::SetEventSystem(EventSystem* system) {
	m_system = system;
}

inline void EventSystem::Update() {
	auto eventsCopy = std::move(m_events);
	while (!eventsCopy.empty()) {
		std::any event = std::move(eventsCopy.front());
		for (auto& listener : m_listeners) {
			listener->TryEvent(event);
		}
		eventsCopy.pop();
	}
}

inline void EventSystem::Add(std::shared_ptr<BasicEventListener> listener) {
	m_listeners.insert(listener);
	listener->SetEventSystem(this);
}

inline void EventSystem::Remove(const std::shared_ptr<BasicEventListener>& listener) {
	listener->SetEventSystem(nullptr);
	m_listeners.erase(listener);
}

inline void EventSystem::Clear() {
	m_listeners.clear();
	
}

template <class Event>
void BasicEventListener::Send(Event&& event) {
	if (m_system) {
		m_system->Send(std::forward<Event>(event));
	}
}

template <class Listener, class... AcceptedEvents>
void EventListener<Listener, AcceptedEvents...>::TryEvent(const std::any& event) {
	constexpr bool invocableForAll = (true && ... && std::is_invocable_v<Listener, AcceptedEvents>);
	static_assert(invocableForAll, "Make sure your Listener type can be invoked with all types listed for the EventListener.");

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
void EventSystem::Send(Event&& event) {
	m_events.push(std::any{ std::forward<Event>(event) });
}


} // namespace inl