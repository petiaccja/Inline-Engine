#include "Scheduler.hpp"

#include <cassert>
#include <iostream> // only for debugging

namespace inl {
namespace gxeng {

void Scheduler::SetPipeline(Pipeline&& pipeline) {
	m_pipeline = std::move(pipeline);
}

const Pipeline& Scheduler::GetPipeline() const {
	return m_pipeline;
}

Pipeline Scheduler::ReleasePipeline() {
	return std::move(m_pipeline);
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
			results.push_back(task(ExecutionContext{ context.gxApi, *context.commandAllocatorPool, *context.scratchSpacePool }));
		}
	}

	// send stuff to the GPU
	for (auto& result : results) {
		for (auto& listRecord : result) {
			BasicCommandList::Decomposition decomp = listRecord.list->Decompose();

			auto beforeFence = context.commandQueue->IncrementFenceValue();
			auto afterFence = context.commandQueue->IncrementFenceValue();
			auto fence = context.commandQueue->GetFence();


			// enqueue make resources resident
			auto makeResidentTask = [resources = decomp.usedResources, list = decomp.commandList.get()]{
				std::cout << "Making stuff resident for " << list << std::endl;
			};
			std::unique_lock<std::mutex> initLkg(*context.initMutex);
			context.initQueue->push({ makeResidentTask, fence, beforeFence });
			initLkg.unlock();
			context.initCv->notify_all();


			// enqueue command list
			dynamic_cast<gxapi::ICopyCommandList*>(decomp.commandList.get())->Close();

			gxapi::ICommandList* execLists[] = {
				decomp.commandList.get(),
			};
			context.commandQueue->Wait(fence, beforeFence);
			context.commandQueue->ExecuteCommandLists(1, execLists);
			context.commandQueue->Signal(fence, afterFence);


			// enqueue clean resources
			auto evictTask = [decomp = std::make_shared<BasicCommandList::Decomposition>(std::move(decomp))]{
				std::cout << "Cleaning stuff after " << decomp->commandList.get() << std::endl;
			};
			std::unique_lock<std::mutex> cleanLkg(*context.cleanMutex);
			context.cleanQueue->push({ std::move(evictTask), fence, afterFence });
			cleanLkg.unlock();
			context.cleanCv->notify_all();
		}
	}
}


void Scheduler::MakeResident(std::vector<GenericResource*> usedResources) {

}

void Scheduler::Evict(std::vector<GenericResource*> usedResources) {

}


}
}
