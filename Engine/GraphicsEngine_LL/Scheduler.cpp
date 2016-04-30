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

void Scheduler::Execute() {
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

	// execute the tasks
	lemon::ListDigraph::NodeMap<bool> nodeInitMap(dependencyGraph, false);
	std::vector<ExecutionResult> results;
	for (auto& taskNode : taskNodes) {
		// initialize parent node if not init already
		lemon::ListDigraph::NodeIt parentGraphNode = taskParentMap[taskNode];
		if (parentGraphNode != lemon::INVALID && !nodeInitMap[parentGraphNode]) {
			exc::NodeBase* parentNode = nodeMap[parentGraphNode];
			//parentNode->Update();
			nodeInitMap[parentGraphNode] = true;
		}

		// execute the task
		ElementaryTask task = taskFunctionMap[taskNode];
		if (task) {
			results.push_back(task(ExecutionContext{}));
		}
	}
}
	
}
}
