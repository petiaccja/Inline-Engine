#pragma once

#include <mutex>
#include <queue>
#include <functional>

#include "SyncPoint.hpp"
#include "CriticalBufferHeap.hpp"
#include "CommandAllocatorPool.hpp"
#include <atomic>


namespace inl {
namespace gxeng {

/// <summary> Manages initializing and cleanup of command lists. </summary>
class ResourceResidencyQueue {
	struct Task {
		Task() = default;
		Task(std::vector<std::shared_ptr<MemoryObject>> resources, SyncPoint syncPoint)
			: resources(std::move(resources)), syncPoint(syncPoint) {}
		virtual ~Task() {};
		std::vector<std::shared_ptr<MemoryObject>> resources;
		SyncPoint syncPoint;
	};
public:
	ResourceResidencyQueue(std::unique_ptr<gxapi::IFence> fence);
	~ResourceResidencyQueue();


	/// <summary> The failure handler will be called if resources cannot be made resident, no matter how hard it tries. </summary>
	/// <remarks> Note that the handler may be called from any thread. Be safe kids, use protection. </summary>
	void SetFailureHandler(std::function<void()> handler);

	/// <summary> I have no idea what you can do with an std::function, but here you go! </summary>
	/// <returns> The currently used failure handler. </returns>
	const std::function<void()>& GetFailureHandler() const;


	/// <summary> Enqueue a list of resources which should be made usable by the GPU by the time the
	///			  returned sync point is signaled. </summary>
	/// <param name="resources"> The resources to be made available resident on GPU memory. </param>
	/// <returns> The SyncPoint returned will be signaled when the requested resources are all resident. </returns>
	SyncPoint EnqueueInit(std::vector<std::shared_ptr<MemoryObject>> resources);

	/// <summary> Enqueue a list of resources which should be marked as evictable. 
	///			  Their memory may be made unresident if more space is needed on the GPU. </summary>
	/// <param name="waitFor"> The resources will only be marked evictable after the SyncPoint is signaled. </param>
	/// <param name="resources"> The list of resources to be evicted. </param>
	/// <param name="cleanObjects"> Object that should live until 'waitFor' is signaled. The destructor of given objects
	///								will be called afterwards. </param>
	/// <remarks> The cleanObjects list could be used to free up command lists and command allocators associated 
	///			  with the resources. </remarks>
	template <class... CleanObjectT>
	void EnqueueClean(SyncPoint waitFor, std::vector<std::shared_ptr<MemoryObject>> resources, CleanObjectT&&... cleanObjects);

private:
	void InitThreadFunc();
	void CleanThreadFunc();
	
private:
	// Init
	std::mutex m_initMutex;
	std::thread m_initThread;
	std::condition_variable m_initCv;
	std::queue<std::unique_ptr<Task>> m_initQueue;

	// Clean
	std::mutex m_cleanMutex;
	std::thread m_cleanThread;
	std::condition_variable m_cleanCv;
	std::queue<std::unique_ptr<Task>> m_cleanQueue;

	// Thread run flag
	std::atomic_bool m_runThreads;

	// Failure avoidance and handling
	std::condition_variable m_retryCv;
	std::function<void()> m_failureHandler;

	// Event tracking
	std::shared_ptr<gxapi::IFence> m_fence;
	uint64_t m_fenceValue;
};


template<class... CleanObjectT>
inline void ResourceResidencyQueue::EnqueueClean(SyncPoint waitFor, std::vector<std::shared_ptr<MemoryObject>> resources, CleanObjectT&&... cleanObjects) {
	struct SpecialTask : Task {
		SpecialTask() = default;
		SpecialTask(std::vector<std::shared_ptr<MemoryObject>> resources, SyncPoint syncPoint, CleanObjectT&&... cleanObjects)
			: Task(std::move(resources), std::move(syncPoint)), data(std::forward<CleanObjectT>(cleanObjects)...) {}
		std::tuple<CleanObjectT...> data;
 	};

	std::lock_guard<std::mutex> lkg(m_cleanMutex);
	m_cleanQueue.push(std::make_unique<SpecialTask>(std::move(resources), std::move(waitFor), std::forward<CleanObjectT>(cleanObjects)...));
	m_cleanCv.notify_one();
}

} // namespace gxeng
} // namespace inl