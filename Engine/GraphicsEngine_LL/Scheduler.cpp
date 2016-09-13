#include "Scheduler.hpp"

#include <cassert>
#include <iostream> // only for debugging
#include "GraphicsApi_D3D12/DescriptorHeap.hpp"

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

	// Topologically sort the tasks.
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

	// Execute the tasks in topological order.
	try {
		for (auto& taskNode : taskNodes) {
			// Execute the task on the CPU.
			ElementaryTask task = taskFunctionMap[taskNode];
			ExecutionResult result;
			if (task) {
				result = task(ExecutionContext{ &context });
			}
			else {
				continue;
			}

			// Enqueue all command lists on the GPU.
			for (ExecutionResult::CommandListRecord& listRecord : result) {
				EnqueueCommandList(*context.commandQueue, std::move(*listRecord.list), context);
			}
		}
	}
	catch (std::exception& ex) {
		// One of the pipeline Nodes (Tasks) threw an exception.
		// Scene cannot be rendered, but we should draw an error message on the screen for the devs.

		// Log error.
		context.log->Event(std::string("Fatal pipeline error: ") + ex.what());

		// Draw a red blinking background to signal error.
		try {
			RenderFailureScreen(context);
		}
		catch (std::exception& ex) {
			context.log->Event(std::string("Fatal pipeline error, could not render error screen: ") + ex.what());
		}
	}
}


void Scheduler::MakeResident(std::vector<GenericResource*> usedResources) {

}


void Scheduler::Evict(std::vector<GenericResource*> usedResources) {

}


void Scheduler::EnqueueCommandList(CommandQueue & commandQueue,
								   BasicCommandList commandList,
								   const FrameContext& context)
{
	// Decompose the command list for further processing.
	BasicCommandList::Decomposition decomp = commandList.Decompose();


	// Acquire fences.
	uint64_t beforeFenceValue = context.commandQueue->IncrementFenceValue();
	uint64_t afterFenceValue = context.commandQueue->IncrementFenceValue();
	gxapi::IFence* fence = context.commandQueue->GetFence();


	// Enqueue CPU task to make resources resident before the command list runs.
	// Sets the 'beforeFenceValue' to let the command list execute.
	auto makeResidentTask = [log = context.log, resources = decomp.usedResources, list = decomp.commandList.get()]{
		std::stringstream ss;
		ss << "Making stuff resident for " << list;
		log->Event(ss.str());
	};
	// Push task to queue.
	std::unique_lock<std::mutex> initLkg(*context.initMutex);
	context.initQueue->push({ makeResidentTask, fence, beforeFenceValue });
	initLkg.unlock();
	context.initCv->notify_all();


	// Enqueue the command list itself on the GPU.
	// Wait for 'beforeFenceValue' to be set.
	// Sets 'afterFenceValue' when finished.
	dynamic_cast<gxapi::ICopyCommandList*>(decomp.commandList.get())->Close();

	gxapi::ICommandList* execLists[] = {
		decomp.commandList.get(),
	};
	context.commandQueue->Wait(fence, beforeFenceValue);
	context.commandQueue->ExecuteCommandLists(1, execLists);
	context.commandQueue->Signal(fence, afterFenceValue);


	// Enqueue CPU task to clean up resources after command list finished.
	// Wait for 'afterFenceValue' to be set.
	auto evictTask = [log = context.log, decomp = std::make_shared<BasicCommandList::Decomposition>(std::move(decomp))]{
		std::stringstream ss;
		ss << "Cleaning stuff after " << decomp->commandList.get();
		log->Event(ss.str());
	};
	// Push task to queue.
	std::unique_lock<std::mutex> cleanLkg(*context.cleanMutex);
	context.cleanQueue->push({ std::move(evictTask), fence, afterFenceValue });
	cleanLkg.unlock();
	context.cleanCv->notify_all();
}


void Scheduler::RenderFailureScreen(FrameContext context) {
	std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(context.absoluteTime);
	int colorMultiplier = elapsed.count() / 800 % 2;
	gxapi::ColorRGBA color{ 0.87f * colorMultiplier, 0, 0 };

	auto commandAllocator = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
	std::unique_ptr<gxapi::IGraphicsCommandList> commandList(context.gxApi->CreateGraphicsCommandList(gxapi::CommandListDesc{ commandAllocator.get() }));

	gxapi::DescriptorHandle rtvHandle = context.backBuffer->GetHandle();
	commandList->SetRenderTargets(1, &rtvHandle);
	commandList->ClearRenderTarget(rtvHandle, color);
	commandList->Close();

	uint64_t afterFenceValue = context.commandQueue->IncrementFenceValue();
	gxapi::IFence* fence = context.commandQueue->GetFence();

	// Enqueue command list
	gxapi::ICommandList* execLists[] = {
		commandList.get(),
	};
	context.commandQueue->ExecuteCommandLists(1, execLists);
	context.commandQueue->Signal(fence, afterFenceValue);

	// Push task only to delay release of command allocator.
	auto evictTask = [log = context.log, commandAllocator = std::shared_ptr<gxapi::ICommandAllocator>(std::move(commandAllocator))]{
		// empty on prupose
	};
	std::unique_lock<std::mutex> cleanLkg(*context.cleanMutex);
	context.cleanQueue->push({ std::move(evictTask), fence, afterFenceValue });
	cleanLkg.unlock();
	context.cleanCv->notify_all();
}



} // namespace gxeng
} // namespace inl
