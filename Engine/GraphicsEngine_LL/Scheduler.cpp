#include "Scheduler.hpp"


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
	lemon::ListDigraph::NodeMap<int> orderMap(taskGraph);
	lemon::checkedTopologicalSort(taskGraph, orderMap);
}
	
}
}
