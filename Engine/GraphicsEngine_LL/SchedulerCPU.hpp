#pragma once

#include "FrameContext.hpp"
#include "Pipeline.hpp"

#include <BaseLibrary/JobSystem/Scheduler.hpp>

#include <any>
#include <optional>



namespace inl::gxeng {


class SchedulerGPU;


class SchedulerCPU {
public:
	void SetPipeline(const Pipeline& pipeline);
	void SetJobScheduler(jobs::Scheduler& scheduler);

	void RunPipeline(const FrameContext& frameContext, SchedulerGPU& schedulerGpu);

private:
	// Refactored way
	struct ProducedCommands {
		std::unique_ptr<BasicCommandList> list;
		std::unique_ptr<VolatileViewHeap> vheap;
	};
	struct FrameContextEx : public FrameContext {
		SchedulerGPU* schedulerGpu = nullptr;
	};

	static std::vector<lemon::ListDigraph::Node> GetSourceNodes(const lemon::ListDigraph& graph);
	void LaunchTasks(const FrameContextEx& context, std::function<jobs::Future<std::any>(const FrameContextEx&, const Pipeline&, lemon::ListDigraph::Node, std::any)> onNode);

	static jobs::Future<std::any> OnSetupNode(const FrameContextEx& context, const Pipeline& pipeline, lemon::ListDigraph::Node node, std::any);
	static void SetupNode(GraphicsTask& task, const FrameContextEx& context);

	static jobs::Future<std::any> OnExecuteNode(const FrameContextEx& context, const Pipeline& pipeline, lemon::ListDigraph::Node node, std::any forwarded);
	static ProducedCommands ExecuteNode(GraphicsTask& task, std::optional<ProducedCommands>& inherited, const FrameContextEx& context);

private:
	const Pipeline* m_pipeline = nullptr;
	jobs::Scheduler* m_scheduler = nullptr;
};



} // namespace inl::gxeng