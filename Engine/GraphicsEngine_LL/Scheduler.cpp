#include "Scheduler.hpp"

#include <cassert>
#include <new>

namespace inl::gxeng {


Scheduler::Scheduler() : m_cpuScheduler(m_pipeline) {
	m_gpuScheduler.SetPipeline(m_pipeline);
	m_gpuScheduler.SetJobScheduler(m_jobScheduler);
}

void Scheduler::SetPipeline(Pipeline&& pipeline) {
	m_pipeline = std::move(pipeline);
	// TODO: Ugly hack...
	m_cpuScheduler.~SchedulerCPU();
	new (&m_cpuScheduler) SchedulerCPU{ m_pipeline };
	m_gpuScheduler.SetPipeline(m_pipeline);
}


const Pipeline& Scheduler::GetPipeline() const {
	return m_pipeline;
}


Pipeline Scheduler::ReleasePipeline() {
	return std::move(m_pipeline);
}


void Scheduler::ReleaseResources() {
	for (NodeBase& node : m_pipeline) {
		if (GraphicsNode* ptr = dynamic_cast<GraphicsNode*>(&node)) {
			ptr->Reset();
		}
	}
}

void Scheduler::Execute(FrameContext context) {
	try {
		m_cpuScheduler.RunPipeline(context, m_jobScheduler, m_gpuScheduler);
	}
	catch (Exception& ex) {
		std::cout << "=== This frame is fucked ===" << std::endl;
		std::cout << "Error message: " << std::endl
				  << ex.what() << std::endl;

		std::cout << std::endl;
		Exception::BreakOnce();
	}
	catch (std::exception& ex) {
		std::cout << "=== This frame is fucked for unknown reasons ===" << std::endl;
		std::cout << "Error message:" << std::endl
				  << ex.what() << std::endl;
	}
}


} // namespace inl::gxeng