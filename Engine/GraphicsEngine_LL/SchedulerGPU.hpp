#pragma once

#include "BasicCommandList.hpp"
#include "FrameContext.hpp"
#include "Pipeline.hpp"
#include "VolatileViewHeap.hpp"

#include <BaseLibrary/JobSystem/ConditionVariable.hpp>
#include <BaseLibrary/JobSystem/SharedFuture.hpp>
#include <BaseLibrary/JobSystem/Scheduler.hpp>

#include <memory>
#include <queue>


namespace inl::gxeng {


struct RenderCommand;
class SchedulerCPU;


class SchedulerGPU {
public:
	SchedulerGPU(Pipeline& pipeline);
	
	void RunPipeline(const FrameContext& frameContext, jobs::Scheduler& scheduler, const SchedulerCPU& cpuScheduler);
	static std::vector<lemon::ListDigraph::Node> SortNodes(const lemon::ListDigraph& taskGraph);

private:
	struct UsedResource {
		MemoryObject* resource;
		unsigned subresource;
		float firstState;
		bool multipleUse;
	};

private:
	jobs::SharedFuture<void> EnqueueCommands(const FrameContext& frameContext, const SchedulerCPU& cpuScheduler);
	RenderCommand UploadResources(const FrameContext& frameContext);

private:
	const Pipeline& m_pipeline;
};



} // namespace inl::gxeng