#include "PipelineEventDispatcher.hpp"
#include <BaseLibrary/ThreadName.hpp>


namespace inl {
namespace gxeng {



PipelineEventDispatcher::PipelineEventDispatcher() {
	state.reset(new State());
	state->m_runDeviceThread = true;
	state->m_runEventThread = true;
	m_deviceSyncThread = std::thread(&PipelineEventDispatcher::DeviceSyncThread, state);
	m_eventThread = std::thread(&PipelineEventDispatcher::DispatchThread, state);
}

PipelineEventDispatcher::PipelineEventDispatcher(PipelineEventDispatcher&& rhs) {
	state = std::move(rhs.state);
	m_eventThread = std::move(rhs.m_eventThread);
	m_deviceSyncThread = std::move(rhs.m_deviceSyncThread);
}

PipelineEventDispatcher& PipelineEventDispatcher::operator=(PipelineEventDispatcher&& rhs) {
	Shutdown();
	
	state = std::move(rhs.state);
	m_eventThread = std::move(rhs.m_eventThread);
	m_deviceSyncThread = std::move(rhs.m_deviceSyncThread);

	return *this;
}

PipelineEventDispatcher::~PipelineEventDispatcher() noexcept {
	Shutdown();
}


void PipelineEventDispatcher::Shutdown() {
	if (m_deviceSyncThread.joinable()) {
		state->m_runDeviceThread = false;
		state->m_deviceCv.notify_all();
		m_deviceSyncThread.join();
	}

	if (m_eventThread.joinable()) {
		state->m_runEventThread = false;
		state->m_eventCv.notify_all();
		m_eventThread.join();
	}
}



std::future<void> PipelineEventDispatcher::DispatchFrameBegin(uint64_t frameId) {
	EventAction eventAction;
	eventAction.action = std::bind(&PipelineEventListener::OnFrameBeginHost, std::placeholders::_1, frameId);
	auto ret = eventAction.signal.get_future();
	PushEventAction(state.get(), std::move(eventAction));
	return ret;
}

std::future<void> PipelineEventDispatcher::DispatchFrameEnd(uint64_t frameId) {
	EventAction eventAction;
	eventAction.action = std::bind(&PipelineEventListener::OnFrameCompleteHost, std::placeholders::_1, frameId);
	auto ret = eventAction.signal.get_future();
	PushEventAction(state.get(), std::move(eventAction));
	return ret;
}


std::future<void> PipelineEventDispatcher::DispatchDeviceFrameBegin(SyncPoint deviceEvent, uint64_t frameId) {
	DeviceEventAction action;
	action.action = std::bind(&PipelineEventListener::OnFrameBeginDevice, std::placeholders::_1, frameId);
	action.premise = deviceEvent;

	std::future<void> ret = action.signal.get_future();

	std::unique_lock<std::mutex>  lkg(state->m_deviceEventMutex);
	state->m_deviceEventActions.push(std::move(action));
	state->m_deviceCv.notify_all();

	return ret;
}

std::future<void> PipelineEventDispatcher::DispatchDeviceFrameEnd(SyncPoint deviceEvent, uint64_t frameId) {
	DeviceEventAction action;
	action.action = std::bind(&PipelineEventListener::OnFrameCompleteDevice, std::placeholders::_1, frameId);
	action.premise = deviceEvent;

	std::future<void> ret = action.signal.get_future();

	std::unique_lock<std::mutex>  lkg(state->m_deviceEventMutex);
	state->m_deviceEventActions.push(std::move(action));
	state->m_deviceCv.notify_all();

	return ret;
}


void PipelineEventDispatcher::operator+=(PipelineEventListener* listener) {
	std::lock_guard<std::mutex> lkg(state->m_listenerMutex);
	state->m_listeners.insert(listener);
}


void PipelineEventDispatcher::operator-=(PipelineEventListener* listener) {
	std::lock_guard<std::mutex> lkg(state->m_listenerMutex);
	state->m_listeners.erase(listener);
}


void PipelineEventDispatcher::DispatchThread(std::shared_ptr<State> state) {
	SetCurrentThreadName("Event Dispatcher Thread");

	while (state->m_runEventThread) {
		// 0. acquire unique lock
		std::unique_lock<std::mutex> eventLock(state->m_eventMutex);

		// 1. wait on any of: 
		//	- first event (condvar)
		//	- first device event (SyncPoint - cannot be changed)
		//	- cancel event (condvar)
		state->m_eventCv.wait(eventLock, [&state] { return !state->m_eventActions.empty() || !state->m_runEventThread; });

		// 2. acquire lock
		// wait re-acquires lock

		// 3. exctract & remove all data from queue
		std::vector<EventAction> events;
		while (!state->m_eventActions.empty()) {
			events.push_back(std::move(state->m_eventActions.front()));
			state->m_eventActions.pop();
		}

		// 4. release lock
		eventLock.unlock();

		// 5. lock collection of event listeners
		std::lock_guard<std::mutex> listenerLock(state->m_listenerMutex);

		// 6. call event handlers
		for (auto& event : events) {
			for (auto listener : state->m_listeners) {
				try {
					event.action(listener);
					event.signal.set_value();
				}
				catch (...) {
					assert(false); // should log instead
					event.signal.set_exception(std::current_exception());
				}
			}
		}

		// 7. ???
		// 8. profit
	}
}

void PipelineEventDispatcher::DeviceSyncThread(std::shared_ptr<State> state) {
	SetCurrentThreadName("Event Dispatcher: Device Sync Thread");

	while (state->m_runDeviceThread) {
		std::unique_lock<std::mutex> lk(state->m_deviceEventMutex);
		state->m_deviceCv.wait(lk, [&state] { return !state->m_deviceEventActions.empty() || !state->m_runDeviceThread; });

		std::vector<DeviceEventAction> events;
		while (!state->m_deviceEventActions.empty()) {
			events.push_back(std::move(state->m_deviceEventActions.front()));
			state->m_deviceEventActions.pop();
		}

		lk.unlock();

		for (auto& event : events) {
			event.premise.Wait();
			PushEventAction(state.get(), EventAction{ std::move(event.action), std::move(event.signal) });
		}
	}
}



void PipelineEventDispatcher::PushEventAction(State* state, EventAction eventAction) {
	std::unique_lock<std::mutex>  lkg(state->m_eventMutex);
	state->m_eventActions.push(std::move(eventAction));
	state->m_eventCv.notify_all();
}



} // namespace gxeng
} // namespace inl