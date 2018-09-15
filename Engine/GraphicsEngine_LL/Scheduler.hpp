#pragma once

#include "FrameContext.hpp"
#include "Pipeline.hpp"
#include "SchedulerCPU.hpp"
#include "SchedulerGPU.hpp"

#include <BaseLibrary/JobSystem/ThreadpoolScheduler.hpp>

namespace inl::gxeng {


class Scheduler {
public:
	Scheduler();

	/// <summary> Currently active pipeline contains the nodes that are executed each frame. </summary>
	/// <remarks> The pipeline cannot be modified outside the scheduler, hence the exclusive access. </remarks>
	void SetPipeline(Pipeline&& pipeline);

	/// <summary> You can read information about currently used pipeline. </summary>
	const Pipeline& GetPipeline() const;

	/// <summary> You can regain ownership of the pipeline and leave the scheduler with an empty pipeline. </summary>
	Pipeline ReleasePipeline();

	/// <summary> Runs the currently bound pipeline nodes using information about the frame. </summary>
	void Execute(FrameContext context);

	/// <summary> Instructs all pipeline nodes to release their resources related to rendering. </summary>
	/// <remarks> This can be called to free resources before resizing the swapchain.
	///		First, references to the swapchain are dropped, second, GPU memory will be freed
	///		so that old resources won't prevent new ones from being allocated. </remarks>
	void ReleaseResources();

private:
	SchedulerCPU m_cpuScheduler;
	SchedulerGPU m_gpuScheduler;
	Pipeline m_pipeline;
	jobs::ThreadpoolScheduler m_jobScheduler;
};


} // namespace inl::gxeng
