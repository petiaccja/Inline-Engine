#pragma once

#include "GraphicsNode.hpp"
#include "Pipeline.hpp"
#include "FrameContext.hpp"

#include <BaseLibrary/optional.hpp>
#include <GraphicsApi_LL/IFence.hpp>
#include <memory>
#include <cstdint>

namespace inl {
namespace gxeng {



class Scheduler {
public:
	Scheduler();

	// don't let anyone else 'own' the pipeline
	void SetPipeline(Pipeline&& pipeline);
	const Pipeline& GetPipeline() const;
	Pipeline ReleasePipeline();
	void Execute(FrameContext context);
protected:
	struct UsedResource {
		GenericResource* resource;
		unsigned subresource;
		float firstState;
		bool multipleUse;
	};


	static void MakeResident(std::vector<GenericResource*> usedResources);
	static void Evict(std::vector<GenericResource*> usedResources);

	static void UploadTask(CopyCommandList& commandList, const std::vector<UploadHeap::UploadDescription>& uploads);

	static std::vector<ElementaryTask> MakeSchedule(const lemon::ListDigraph& taskGraph,
													const lemon::ListDigraph::NodeMap<ElementaryTask>& taskFunctionMap
													/*std::vector<CommandQueue*> queues*/);

	static void EnqueueCommandList(CommandQueue& commandQueue,
								   std::unique_ptr<gxapi::ICopyCommandList> commandList,
								   CmdAllocPtr commandAllocator,
								   std::vector<std::shared_ptr<GenericResource>> usedResources,
								   const FrameContext& context);

	template <class UsedResourceIter>
	static std::vector<gxapi::ResourceBarrier> Scheduler::InjectBarriers(UsedResourceIter firstResource, UsedResourceIter lastResource);

	template <class UsedResourceIter1, class UsedResourceIter2>
	static bool CanExecuteParallel(UsedResourceIter1 first1, UsedResourceIter1 last1, UsedResourceIter2 first2, UsedResourceIter2 last2);

	template <class UsedResourceIter>
	static void UpdateResourceStates(UsedResourceIter firstResource, UsedResourceIter lastResource);

	static void RenderFailureScreen(FrameContext context);
private:
	Pipeline m_pipeline;
};



template <class UsedResourceIter>
std::vector<gxapi::ResourceBarrier> Scheduler::InjectBarriers(UsedResourceIter firstResource, UsedResourceIter lastResource) {
	std::vector<gxapi::ResourceBarrier> barriers;

	// Collect all necessary barriers.
	for (UsedResourceIter it = firstResource; it != lastResource; ++it) {
		GenericResource* resource = it->resource.get();
		unsigned subresource = it->subresource;
		gxapi::eResourceState targetState = it->firstState;

		gxapi::eResourceState sourceState = resource->ReadState(subresource);

		if (sourceState != targetState) {
			barriers.push_back(gxapi::TransitionBarrier{ resource->_GetResourcePtr(), sourceState, targetState, subresource });
		}
	}

	return barriers;
}


template <class UsedResourceIter1, class UsedResourceIter2>
bool Scheduler::CanExecuteParallel(UsedResourceIter1 first1, UsedResourceIter1 last1, UsedResourceIter2 first2, UsedResourceIter2 last2) {
	UsedResourceIter1 it1 = first1;
	UsedResourceIter2 it2 = first2;

	// Advance the two iterators on the sorted ranges simultaneously.
	while (it1 != last1 && it2 != last2) {
		if (it1->resource < it2->resource) {
			++it1;
		}
		else if (it1->resource > it2->resource) {
			++it2;
		}
		else {
			// If the resources are the same, but uses are incompatible, return false.
			if (it1->firstState != it2->firstState
				|| it1->multipleUse
				|| it2->multipleUse)
			{
				return false;
			}
			++it1;
			++it2;
		}
	}

	return true;
}


template <class UsedResourceIter>
void Scheduler::UpdateResourceStates(UsedResourceIter firstResource, UsedResourceIter lastResource) {
	for (auto it = firstResource; it != lastResource; ++it) {
		GenericResource* resource = it->resource.get();
		resource->RecordState(it->subresource, it->lastState);
	}
}


} // namespace gxeng
} // namespace inl
