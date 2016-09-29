#include "Scheduler.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>

#include <cassert>
#include <iostream> // only for debugging

namespace inl {
namespace gxeng {


Scheduler::Scheduler()
	{}

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
					CmdAllocPtr injectAlloc = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
					std::unique_ptr<gxapi::ICopyCommandList> injectList(context.gxApi->CreateGraphicsCommandList({ injectAlloc.get() }));

					injectList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
					injectList->Close();

					EnqueueCommandList(*context.commandQueue,
						std::move(injectList),
						std::move(injectAlloc),
						{},
						context);
				}

				// Enqueue actual command list.
				std::vector<std::shared_ptr<GenericResource>> usedResourceList;
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

		// Set backBuffer to PRESENT state.
		CmdAllocPtr injectAlloc = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
		std::unique_ptr<gxapi::ICopyCommandList> injectList(context.gxApi->CreateGraphicsCommandList({ injectAlloc.get() }));

		injectList->ResourceBarrier(gxapi::TransitionBarrier{
			context.backBuffer->_GetResourcePtr(),
			context.backBuffer->ReadState(0),
			gxapi::eResourceState::PRESENT});
		injectList->Close();

		EnqueueCommandList(*context.commandQueue,
			std::move(injectList),
			std::move(injectAlloc),
			{},
			context);
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
	std::vector<std::shared_ptr<GenericResource>> usedResources,
	const FrameContext& context)
{
	// Enqueue CPU task to make resources resident before the command list runs.
	SyncPoint residentPoint = context.residencyQueue->EnqueueInit(usedResources);

	// Enqueue the command list itself on the GPU.
	gxapi::ICommandList* execLists[] = {
		commandList.get(),
	};
	context.commandQueue->Wait(residentPoint);
	context.commandQueue->ExecuteCommandLists(1, execLists);
	SyncPoint completionPoint = context.commandQueue->Signal();

	// Enqueue CPU task to clean up resources after command list finished.
	context.residencyQueue->EnqueueClean(completionPoint, std::move(usedResources), std::move(commandAllocator));
}


void Scheduler::RenderFailureScreen(FrameContext context) {
	// Decide wether to show blinking image.
	std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(context.absoluteTime);
	int colorMultiplier = elapsed.count() / 400 % 2;
	gxapi::ColorRGBA color{ 0.87f * colorMultiplier, 0, 0 };

	// Create command allocator & list.
	auto commandAllocator = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
	std::unique_ptr<gxapi::IGraphicsCommandList> commandList(context.gxApi->CreateGraphicsCommandList(gxapi::CommandListDesc{ commandAllocator.get() }));

	gxapi::DescriptorHandle rtvHandle = context.backBuffer->GetHandle();

	// Transition backbuffer to RTV.
	if (context.backBuffer->ReadState(0) != gxapi::eResourceState::RENDER_TARGET) {
		commandList->ResourceBarrier(gxapi::TransitionBarrier(
			context.backBuffer->_GetResourcePtr(),
			context.backBuffer->ReadState(0),
			gxapi::eResourceState::RENDER_TARGET,
			0));
	}

	// Set RTV.
	commandList->SetRenderTargets(1, &rtvHandle);

	// Draw image.
	int width = (int)context.backBuffer->GetWidth();
	int height = (int)context.backBuffer->GetHeight();
	commandList->ClearRenderTarget(rtvHandle, gxapi::ColorRGBA{ 0.2f, 0.2f, 0.2f });
	std::vector<gxapi::Rectangle> rects;
	for (float t = 0.2f; t < 0.8005f; t += 0.05f) {
		int cx = int(t * width);
		int cy = int(t * height);
		gxapi::Rectangle rect;
		rect.top = cy - height / 16;
		rect.bottom = cy + height / 16;
		rect.left = cx - width / 16;
		rect.right = cx + width / 16;
		rects.push_back(rect);
	}
	commandList->ClearRenderTarget(rtvHandle, color, rects.size(), rects.data());
	rects.clear();
	for (float t = 0.2f; t < 0.8005f; t += 0.05f) {
		int cx = int(t * width);
		int cy = int((1 - t) * height);
		gxapi::Rectangle rect;
		rect.top = cy - height / 16;
		rect.bottom = cy + height / 16;
		rect.left = cx - width / 16;
		rect.right = cx + width / 16;
		rects.push_back(rect);
	}
	commandList->ClearRenderTarget(rtvHandle, color, rects.size(), rects.data());


	// Transition backbuffer to PRESENT.
	commandList->ResourceBarrier(gxapi::TransitionBarrier(
		context.backBuffer->_GetResourcePtr(),
		gxapi::eResourceState::RENDER_TARGET,
		gxapi::eResourceState::PRESENT,
		0));
	context.backBuffer->RecordState(0, gxapi::eResourceState::PRESENT);

	// Enqueue command list.
	commandList->Close();
	EnqueueCommandList(*context.commandQueue, std::move(commandList), std::move(commandAllocator), {}, context);
}



} // namespace gxeng
} // namespace inl
