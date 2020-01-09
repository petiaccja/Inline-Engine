#pragma once

#include "FrameContext.hpp"
#include "Pipeline.hpp"

#include "BaseLibrary/JobSystem/Mutex.hpp"
#include <BaseLibrary/JobSystem/Scheduler.hpp>

namespace inl::gxeng {


struct RenderCommand {
	std::unique_ptr<BasicCommandList> list;
	std::unique_ptr<VolatileViewHeap> vheap;
};


class SchedulerCPU {
	struct RenderCommandCandidate {
		std::unique_ptr<BasicCommandList> inheritedList;
		std::unique_ptr<VolatileViewHeap> inheritedVheap;
		std::unique_ptr<BasicCommandList> list;
		std::unique_ptr<VolatileViewHeap> vheap;
	};

public:
	SchedulerCPU(const Pipeline& pipeline);
	SchedulerCPU(SchedulerCPU&& rhs);

	void RunPipeline(const FrameContext& frameContext, jobs::Scheduler& scheduler);

	auto GetCommandJobs() const -> const lemon::ListDigraph::NodeMap<std::shared_ptr<jobs::SharedFuture<RenderCommand>>>&;

private:
	void CalculateListForwarding();
	void FindTaskGraphSinks();

	void CreateSetupJobs(const FrameContext& frameContext, jobs::Scheduler& scheduler);
	void CreateExecuteJobs(const FrameContext& frameContext, jobs::Scheduler& scheduler);
	void CreateCommandJobs(jobs::Scheduler& scheduler);

	jobs::SharedFuture<void> SetupJob(lemon::ListDigraph::Node node, const FrameContext& frameContext);
	jobs::SharedFuture<RenderCommandCandidate> ExecuteJob(lemon::ListDigraph::Node node, const FrameContext& frameContext);
	jobs::SharedFuture<RenderCommand> CommandJob(lemon::ListDigraph::Node node);

private:
	const Pipeline& m_pipeline;
	lemon::ListDigraph::ArcMap<bool> m_listForwarding;
	std::vector<lemon::ListDigraph::Node> m_sinks;
	lemon::ListDigraph::NodeMap<std::shared_ptr<jobs::SharedFuture<void>>> m_setupJobs;
	lemon::ListDigraph::NodeMap<std::shared_ptr<jobs::SharedFuture<RenderCommandCandidate>>> m_executeJobs;
	lemon::ListDigraph::NodeMap<std::shared_ptr<jobs::SharedFuture<RenderCommand>>> m_commandJobs;
};



} // namespace inl::gxeng