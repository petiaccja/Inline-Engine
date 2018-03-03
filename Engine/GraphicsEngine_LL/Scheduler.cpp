#include "Scheduler.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <BaseLibrary/AtScopeExit.hpp>

#include "GraphicsCommandList.hpp"

#include <cassert>
#include <typeinfo> // Track render passes.
#include <regex> // Node typeinfo name prettyfy.
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
	const auto& taskFunctionMap = m_pipeline.GetTaskFunctionMap();

	auto tasks = MakeSchedule(taskGraph, taskFunctionMap);

	// Inject copy task to the start.
	UploadTask uploadTask(context.uploadRequests);
	tasks.insert(tasks.begin(), &uploadTask);

	// Setup and execute the tasks.
	try {
		// PHASE I.: Setup() tasks in correct order
		for (auto& task : tasks) {
			if (task != nullptr) {
				SetupContext setupContext(context.memoryManager, context.textureSpace, context.rtvHeap, context.dsvHeap, context.shaderManager, context.gxApi);
				task->Setup(setupContext);
			}
		}


		// PHASE II.: Execute() tasks in correct
		for (auto& task : tasks) {
			VolatileViewHeap volatileHeap(context.gxApi);
			RenderContext renderContext(context.memoryManager,
										context.textureSpace,
										&volatileHeap,
										context.shaderManager,
										context.gxApi,
										context.commandListPool,
										context.commandAllocatorPool,
										context.scratchSpacePool);

			// Execute the task on the CPU.
			if (task != nullptr) {
				// Mark debug event on command queue.
				std::string className = typeid(*task).name();
				size_t idx = className.find("inl::gxeng::");
				if (idx != className.npos) { className = className.substr(idx+12); }
				std::stringstream passName;
				passName << "Node - " << className;
				context.commandQueue->GetUnderlyingQueue()->BeginDebuggerEvent(passName.str());
				AtScopeExit endPass([&context] { context.commandQueue->GetUnderlyingQueue()->EndDebuggerEvent(); });
				

				// Call execute.
				task->Execute(renderContext);

				// Enqueue all command lists on the GPU.
				if (renderContext.IsListInitialized()) {
					BasicCommandList* commandList = nullptr;
					switch (renderContext.GetType()) {
						case gxapi::eCommandListType::GRAPHICS: commandList = &renderContext.AsGraphics(); break;
						case gxapi::eCommandListType::COMPUTE: commandList = &renderContext.AsCompute(); break;
						case gxapi::eCommandListType::COPY: commandList = &renderContext.AsCopy(); break;
						default: assert(false);
					}
					BasicCommandList::Decomposition decomposition = commandList->Decompose();

					std::sort(decomposition.usedResources.begin(), decomposition.usedResources.end(), [](const ResourceUsage& lhs, const ResourceUsage& rhs) {
						auto lhsPtr = lhs.resource._GetResourcePtr();
						auto rhsPtr = rhs.resource._GetResourcePtr();
						return lhsPtr < rhsPtr || (lhs.resource._GetResourcePtr() == rhs.resource._GetResourcePtr() && lhs.subresource < rhs.subresource);
					});

					// Inject a transition barrier command list.
					auto barriers = InjectBarriers(decomposition.usedResources.begin(), decomposition.usedResources.end());
					if (barriers.size() > 0) {
						CmdAllocPtr injectAlloc = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
						GraphicsCmdListPtr injectList = context.commandListPool->RequestGraphicsList(injectAlloc.get());

						injectList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
						injectList->Close();

						EnqueueCommandList(*context.commandQueue,
										   std::move(injectList),
										   std::move(injectAlloc),
										   {},
										   {},
										   {},
										   context);
					}

					// Update resource states.
					UpdateResourceStates(decomposition.usedResources.begin(), decomposition.usedResources.end());

					// Enqueue actual command list.
					std::vector<MemoryObject> usedResourceList;
					usedResourceList.reserve(decomposition.usedResources.size());
					for (auto& v : decomposition.usedResources) {
						usedResourceList.push_back(std::move(v.resource));
					}
					for (auto& v : decomposition.additionalResources) {
						usedResourceList.push_back(std::move(v));
					}

					dynamic_cast<gxapi::ICopyCommandList*>(decomposition.commandList.get())->Close();

					EnqueueCommandList(*context.commandQueue,
									   std::move(decomposition.commandList),
									   std::move(decomposition.commandAllocator),
									   std::move(decomposition.scratchSpaces),
									   std::move(usedResourceList),
									   std::unique_ptr<VolatileViewHeap>(new VolatileViewHeap(std::move(volatileHeap))),
									   context);



				}
			}
		}

		// Set backBuffer to PRESENT state.
		gxapi::eResourceState bbState = context.backBuffer->GetResource().ReadState(0);

		if (bbState != gxapi::eResourceState::PRESENT) {
			CmdAllocPtr injectAlloc = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
			GraphicsCmdListPtr injectList = context.commandListPool->RequestGraphicsList(injectAlloc.get());

			injectList->ResourceBarrier(gxapi::TransitionBarrier{
				context.backBuffer->GetResource()._GetResourcePtr(),
				context.backBuffer->GetResource().ReadState(0),
				gxapi::eResourceState::PRESENT });
			injectList->Close();
			context.backBuffer->GetResource().RecordState(gxapi::eResourceState::PRESENT);

			EnqueueCommandList(*context.commandQueue,
				std::move(injectList),
				std::move(injectAlloc),
				{},
				{},
				{},
				context);
		}
	}
	catch (std::exception& ex) {
		// One of the pipeline Nodes (Tasks) threw an exception.
		// Scene cannot be rendered, but we should draw an error message on the screen for the devs.

		// Log error.
		context.log->Event(std::string("Fatal pipeline Execute error: ") + ex.what());

		// Draw a red blinking background to signal error.
		try {
			RenderFailureScreen(context);
		}
		catch (std::exception& ex) {
			context.log->Event(std::string("Fatal pipeline Execute error, could not render error screen: ") + ex.what());
		}
	}
}


void Scheduler::ReleaseResources() {
	for (NodeBase& node : m_pipeline) {
		if (GraphicsNode* ptr = dynamic_cast<GraphicsNode*>(&node)) {
			ptr->Reset();
		}
	}
}


void Scheduler::MakeResident(std::vector<MemoryObject*> usedResources) {

}


void Scheduler::Evict(std::vector<MemoryObject*> usedResources) {

}

std::vector<GraphicsTask*> Scheduler::MakeSchedule(const lemon::ListDigraph& taskGraph,
													const lemon::ListDigraph::NodeMap<GraphicsTask*>& taskFunctionMap
/*std::vector<CommandQueue*> queues*/)
{
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

	// Make a list of them.
	std::vector<GraphicsTask*> tasks;
	for (auto node : taskNodes) {
		auto& task = taskFunctionMap[node];
		tasks.push_back(task);
	}

	return tasks;
}


void Scheduler::EnqueueCommandList(CommandQueue& commandQueue,
								   CmdListPtr commandList,
								   CmdAllocPtr commandAllocator,
								   std::vector<ScratchSpacePtr> scratchSpaces,
								   std::vector<MemoryObject> usedResources,
								   std::unique_ptr<VolatileViewHeap> volatileHeap,
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
	context.residencyQueue->EnqueueClean(completionPoint, std::move(usedResources), std::move(commandAllocator), std::move(scratchSpaces), std::move(volatileHeap));
}


void Scheduler::RenderFailureScreen(FrameContext context) {
	// Decide wether to show blinking image.
	std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(context.absoluteTime);
	int colorMultiplier = elapsed.count() / 400 % 2;
	gxapi::ColorRGBA color{ 0.87f * colorMultiplier, 0, 0 };

	// Create command allocator & list.
	auto commandAllocator = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
	auto commandList = context.commandListPool->RequestGraphicsList(commandAllocator.get());

	gxapi::DescriptorHandle rtvHandle = context.backBuffer->GetHandle();

	// Transition backbuffer to RTV.
	if (context.backBuffer->GetResource().ReadState(0) != gxapi::eResourceState::RENDER_TARGET) {
		commandList->ResourceBarrier(gxapi::TransitionBarrier(
			context.backBuffer->GetResource()._GetResourcePtr(),
			context.backBuffer->GetResource().ReadState(0),
			gxapi::eResourceState::RENDER_TARGET,
			0));
	}

	// Set RTV.
	commandList->SetRenderTargets(1, &rtvHandle);

	// Draw image.
	int width = (int)context.backBuffer->GetResource().GetWidth();
	int height = (int)context.backBuffer->GetResource().GetHeight();
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
		context.backBuffer->GetResource()._GetResourcePtr(),
		gxapi::eResourceState::RENDER_TARGET,
		gxapi::eResourceState::PRESENT,
		0));
	context.backBuffer->GetResource().RecordState(0, gxapi::eResourceState::PRESENT);

	// Enqueue command list.
	commandList->Close();
	EnqueueCommandList(*context.commandQueue, std::move(commandList), std::move(commandAllocator), {}, {}, {}, context);
}


void Scheduler::UploadTask::Setup(SetupContext& context) {
	return;
}
void Scheduler::UploadTask::Execute(RenderContext& context) {
	CopyCommandList& commandList = context.AsGraphics();

	for (auto& request : *m_uploads) {
		// Init copy parameters
		auto& source = request.source;
		auto& destination = const_cast<MemoryObject&>(request.destination);

		// Set resource states
		commandList.SetResourceState(destination, gxapi::eResourceState::COPY_DEST);

		auto destType = request.destType;

		if (destType == UploadManager::DestType::BUFFER) {
			auto& dstBuffer = static_cast<LinearBuffer&>(destination);
			commandList.CopyBuffer(dstBuffer, request.dstOffsetX, source, 0, dstBuffer.GetSize());
		}
		else if (destType == UploadManager::DestType::TEXTURE_2D) {
			auto& dstTexture = static_cast<Texture2D&>(destination);
			commandList.CopyTexture(dstTexture, source, SubTexture2D(request.dstSubresource, Vector<intptr_t, 2>((intptr_t)request.dstOffsetX, (intptr_t)request.dstOffsetY)), request.textureBufferDesc);
		}
	}
}



} // namespace gxeng
} // namespace inl
