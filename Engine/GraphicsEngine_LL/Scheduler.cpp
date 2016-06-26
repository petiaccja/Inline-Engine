#include "Scheduler.hpp"

#include <cassert>

namespace inl {
namespace gxeng {

void Scheduler::SetPipeline(Pipeline&& pipeline) {
	m_pipeline = std::move(pipeline);
}

const Pipeline& Scheduler::GetPipeline() const {
	return m_pipeline;
}

void Scheduler::Execute(FrameContext context) {
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	const auto& dependencyGraph = m_pipeline.GetDependencyGraph();
	const auto& taskParentMap = m_pipeline.GetTaskParentMap();
	const auto& nodeMap = m_pipeline.GetNodeMap();
	const auto& taskFunctionMap = m_pipeline.GetTaskFunctionMap();

	// topologically sort the tasks
	lemon::ListDigraph::NodeMap<int> taskOrderMap(taskGraph);
	bool isSortable = lemon::checkedTopologicalSort(taskGraph, taskOrderMap);
	assert(isSortable);

	std::vector<lemon::ListDigraph::NodeIt> taskNodes;
	for (lemon::ListDigraph::NodeIt taskNode(taskGraph); taskNode != lemon::INVALID; ++taskNode) {
		taskNodes.push_back(taskNode);
	}

	std::sort(taskNodes.begin(), taskNodes.end(), [&](auto n1, auto n2)
	{
		return taskOrderMap[n1] < taskOrderMap[n2];
	});

	// execute the tasks in topological order on CPU side
	std::vector<ExecutionResult> results;
	for (auto& taskNode : taskNodes) {
		// execute the task
		ElementaryTask task = taskFunctionMap[taskNode];
		if (task) {
			results.push_back(task(ExecutionContext{*context.commandAllocatorPool}));
		}
	}

	// send stuff to the GPU
	for (auto& result : results) {
		for (auto& listRecord : result) {
			BasicCommandList::Decomposition decomp = listRecord.list->Decompose();

			// TODO:
			// make resources resident

			context.commandQueue->ExecuteCommandLists(1, &decomp.commandList);
			delete decomp.commandList;

			// TODO:
			// get a sync point
			// mark commandAllocator for sync point
			// mark usedResources for sync point			
		}
	}
}


}
}
