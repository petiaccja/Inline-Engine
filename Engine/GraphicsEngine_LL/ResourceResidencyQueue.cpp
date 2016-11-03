#include "ResourceResidencyQueue.hpp"
#include <BaseLibrary/ThreadName.hpp>

namespace inl {
namespace gxeng {


ResourceResidencyQueue::ResourceResidencyQueue(std::unique_ptr<gxapi::IFence> fence) 
	: m_fence(std::move(fence)),
	m_fenceValue(0)
{
	m_fence->Signal(0);
	m_runThreads = true;
	m_initThread = std::thread(std::bind(&ResourceResidencyQueue::InitThreadFunc, this));
	m_cleanThread = std::thread(std::bind(&ResourceResidencyQueue::CleanThreadFunc, this));
}


ResourceResidencyQueue::~ResourceResidencyQueue() {
	m_runThreads = false;
	m_initCv.notify_all();
	m_cleanCv.notify_all();
	m_initThread.join();
	m_cleanThread.join();
}


void ResourceResidencyQueue::SetFailureHandler(std::function<void()> handler) {
	m_failureHandler = handler;
}


const std::function<void()>& ResourceResidencyQueue::GetFailureHandler() const {
	return m_failureHandler;
}


SyncPoint ResourceResidencyQueue::EnqueueInit(std::vector<std::shared_ptr<MemoryObject>> resources) {
	++m_fenceValue;
	SyncPoint syncPoint(m_fence, m_fenceValue);

	std::lock_guard<std::mutex> lkg(m_initMutex);
	m_initQueue.push(std::make_unique<Task>(std::move(resources), syncPoint));
	m_initCv.notify_one();

	return syncPoint;
}


void ResourceResidencyQueue::InitThreadFunc() {
	SetCurrentThreadName("CommandList Init Thread");

	std::vector<std::unique_ptr<Task>> workingSet;
	while (m_runThreads) {
		std::unique_lock<std::mutex> lk(m_initMutex);
		m_initCv.wait(lk, [this] {return !m_runThreads || !m_initQueue.empty(); });

		while (!m_initQueue.empty()) {
			std::unique_ptr<Task> task = std::move(m_initQueue.front());
			workingSet.push_back(std::move(task));
			m_initQueue.pop();
		}
		lk.unlock();

		for (auto& task : workingSet) {
			for (const std::shared_ptr<MemoryObject>& res : task->resources) {
				// TODO...
			}
			task->syncPoint.m_fence->Signal(task->syncPoint.m_value);
		}

		workingSet.clear();
	}
}


void ResourceResidencyQueue::CleanThreadFunc() {
	SetCurrentThreadName("CommandList Clean Thread");

	std::vector<std::unique_ptr<Task>> workingSet;
	while (m_runThreads) {
		std::unique_lock<std::mutex> lk(m_cleanMutex);
		m_cleanCv.wait(lk, [this] {return !m_runThreads || !m_cleanQueue.empty(); });

		while (!m_cleanQueue.empty()) {
			std::unique_ptr<Task> task = std::move(m_cleanQueue.front());
			workingSet.push_back(std::move(task));
			m_cleanQueue.pop();
		}
		lk.unlock();

		for (auto& task : workingSet) {
			task->syncPoint.m_fence->Wait(task->syncPoint.m_value);
			for (const std::shared_ptr<MemoryObject>& res : task->resources) {
				// TODO...
			}
		}

		workingSet.clear();
	}
}


} // namespace gxeng
} // namespace inl