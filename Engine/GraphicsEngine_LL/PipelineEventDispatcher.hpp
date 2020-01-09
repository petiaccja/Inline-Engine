#pragma once

#include "PipelineEventListener.hpp"
#include "SyncPoint.hpp"

#include <cstdint>
#include <future>
#include <mutex>
#include <queue>
#include <set>
#include <thread>


namespace inl::gxeng {


class PipelineEventDispatcher {
	struct EventAction {
		std::function<void(PipelineEventListener*)> action;
		std::promise<void> signal;
	};
	struct DeviceEventAction : EventAction {
		SyncPoint premise; // NOT prOmise
	};
	enum eReasonForAwake {
		CANCEL = 0,
		QUIT = 1,
		EVENT,
	};
	struct State {
		std::queue<EventAction> m_eventActions;
		std::mutex m_eventMutex;
		std::condition_variable m_eventCv;
		std::atomic_bool m_runEventThread;

		std::queue<DeviceEventAction> m_deviceEventActions;
		std::mutex m_deviceEventMutex;
		std::condition_variable m_deviceCv;
		std::atomic_bool m_runDeviceThread;

		std::mutex m_listenerMutex;
		std::set<PipelineEventListener*> m_listeners;
	};

public:
	PipelineEventDispatcher();
	PipelineEventDispatcher(const PipelineEventDispatcher&) = delete;
	PipelineEventDispatcher(PipelineEventDispatcher&&);
	PipelineEventDispatcher& operator=(const PipelineEventDispatcher&) = delete;
	PipelineEventDispatcher& operator=(PipelineEventDispatcher&&);
	~PipelineEventDispatcher() noexcept;


	std::future<void> DispatchFrameBegin(uint64_t frameId);
	std::future<void> DispatchFrameEnd(uint64_t frameId);
	std::future<void> DispachFrameBeginAwait(uint64_t frameId);
	std::future<void> DispatchDeviceFrameBegin(SyncPoint deviceEvent, uint64_t frameId);
	std::future<void> DispatchDeviceFrameEnd(SyncPoint deviceEvent, uint64_t frameId);


	void operator+=(PipelineEventListener* listener);
	void operator-=(PipelineEventListener* listener);

private:
	static void DispatchThread(std::shared_ptr<State> state);
	static void DeviceSyncThread(std::shared_ptr<State> state);
	static void PushEventAction(State* state, EventAction eventAction);
	void Shutdown();

private:
	std::shared_ptr<State> state;

	std::thread m_eventThread;
	std::thread m_deviceSyncThread;
};


} // namespace inl::gxeng