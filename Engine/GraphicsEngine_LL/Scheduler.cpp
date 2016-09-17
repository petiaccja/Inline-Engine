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
				auto dec = listRecord.list->Decompose();

				std::sort(dec.usedResources.begin(), dec.usedResources.end(), [](const ResourceUsage& lhs, const ResourceUsage& rhs) {
					return lhs.resource < rhs.resource || (lhs.resource == rhs.resource && lhs.subresource < rhs.subresource);
				});

				// Inject a transition barrier command list.
				auto barriers = InjectBarriers(dec.usedResources.begin(), dec.usedResources.end());
				if (barriers.size() > 0) {
					CmdAllocPtr injectAlloc = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::COPY);
					std::unique_ptr<gxapi::ICopyCommandList> injectList(context.gxApi->CreateCopyCommandList({ injectAlloc.get() }));

					injectList->ResourceBarrier(barriers.size(), barriers.data());
					injectList->Close();

					EnqueueCommandList(*context.commandQueue,
									   std::move(injectList),
									   std::move(injectAlloc),
									   {},
									   context);
				}

				// Enqueue actual command list.
				std::vector<GenericResource*> usedResourceList;
				usedResourceList.reserve(dec.usedResources.size());
				for (const auto& v : dec.usedResources) {
					usedResourceList.push_back(v.resource);
				}

				dec.commandList->Close();
				EnqueueCommandList(*context.commandQueue,
								   std::move(dec.commandList),
								   std::move(dec.commandAllocator),
								   std::move(usedResourceList),
								   context);


				// Update resource states.
				UpdateResourceStates(dec.usedResources.begin(), dec.usedResources.end());
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


void Scheduler::EnqueueCommandList(CommandQueue& commandQueue,
								   std::unique_ptr<gxapi::ICopyCommandList> commandList,
								   CmdAllocPtr commandAllocator,
								   std::vector<GenericResource*> usedResources,
								   const FrameContext& context)
{
	// Acquire fences.
	gxapi::IFence* fence = context.commandQueue->GetFence();
	uint64_t beforeFenceValue = 0;
	beforeFenceValue = context.commandQueue->IncrementFenceValue();
	uint64_t afterFenceValue = context.commandQueue->IncrementFenceValue();

	// Enqueue CPU task to make resources resident before the command list runs.
	// Sets the 'beforeFenceValue' to let the command list execute.
	auto makeResidentTask = [log = context.log, resources = usedResources, commandList = commandList.get()]{
		// careful, 'commandList' is a dangling pointer
		std::stringstream ss;
		ss << "Making stuff resident for " << commandList;
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
	gxapi::ICommandList* execLists[] = {
		commandList.get(),
	};
	context.commandQueue->Wait(fence, beforeFenceValue);
	context.commandQueue->ExecuteCommandLists(1, execLists);
	context.commandQueue->Signal(fence, afterFenceValue);

	// Enqueue CPU task to clean up resources after command list finished.
	// Wait for 'afterFenceValue' to be set.
	auto evictTask = [log = context.log, resources = usedResources, commandList = commandList.get(), commandAllocator = std::shared_ptr<gxapi::ICommandAllocator>(std::move(commandAllocator))]{
		// careful, 'commandList' is a dangling pointer
		std::stringstream ss;
		ss << "Cleaning stuff after " << commandList;
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

	if (context.backBuffer->ReadState(0) != gxapi::eResourceState::RENDER_TARGET) {
		commandList->ResourceBarrier(gxapi::TransitionBarrier(
			context.backBuffer->_GetResourcePtr(),
			context.backBuffer->ReadState(0),
			gxapi::eResourceState::RENDER_TARGET,
			0));
	}

	commandList->SetRenderTargets(1, &rtvHandle);
	commandList->ClearRenderTarget(rtvHandle, color);

	commandList->ResourceBarrier(gxapi::TransitionBarrier(
		context.backBuffer->_GetResourcePtr(),
		gxapi::eResourceState::RENDER_TARGET,
		gxapi::eResourceState::PRESENT,
		0));
	context.backBuffer->RecordState(0, gxapi::eResourceState::PRESENT);

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
