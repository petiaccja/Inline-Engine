#include "Scheduler.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <BaseLibrary/AtScopeExit.hpp>

#include "GraphicsCommandList.hpp"

#include <cassert>
#include <typeinfo> // Track render passes.
#include <fstream> // only for debugging
#include <sstream>
#include <iomanip>

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


void Scheduler::ReleaseResources() {
	for (NodeBase& node : m_pipeline) {
		if (GraphicsNode* ptr = dynamic_cast<GraphicsNode*>(&node)) {
			ptr->Reset();
		}
	}
}


//--------------------------------------------
// Multi-threaded rendering
//--------------------------------------------

struct NodeDeps {
	NodeDeps() : numSatisfied(std::make_shared<std::atomic_size_t>(0)) {}
	std::shared_ptr<std::atomic_size_t> numSatisfied;
	size_t numTotal;
};


template <class OnNode>
jobs::Task<void> TraverseNode(jobs::Scheduler& scheduler,
				  const lemon::ListDigraph& graph,
				  lemon::ListDigraph::NodeMap<NodeDeps>& dependencyCounter, 
				  lemon::ListDigraph::Node node,
				  OnNode onNode) 
{
	onNode(node);

	for (lemon::ListDigraph::OutArcIt outArc(graph, node); outArc != lemon::INVALID; ++outArc) {
		auto nextNode = graph.target(outArc);
		int numSatisfied = 1 + dependencyCounter[nextNode].numSatisfied->fetch_add(1);
		if (numSatisfied == dependencyCounter[nextNode].numTotal) {
			auto nextTask = scheduler.EnqueueTask(TraverseNode<OnNode>, 
												  std::ref(scheduler),
												  std::ref(graph),
												  std::ref(dependencyCounter),
												  nextNode,
												  onNode);
			co_await nextTask;
		}
	}
}


void Scheduler::ExecuteParallel(FrameContext context) {
	// Get task graph and function map.
	const auto& taskGraph = m_pipeline.GetTaskGraph();
	const auto& taskFunctions = m_pipeline.GetTaskFunctionMap();


	// Get dependency count and source nodes.
	lemon::ListDigraph::NodeMap<NodeDeps> dependencyCounter(taskGraph);
	std::vector<lemon::ListDigraph::Node> sourceNodes;


	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		size_t numDependencies = countInArcs(taskGraph, nodeIt);
		*dependencyCounter[nodeIt].numSatisfied = 0;
		dependencyCounter[nodeIt].numTotal = numDependencies;
		if (numDependencies == 0) {
			sourceNodes.push_back(nodeIt);
		}
	}

	// Launch task for all source nodes.
	struct TraceInfo {
		unsigned depth = 0;
	};
	lemon::ListDigraph::NodeMap<TraceInfo> traceMap(taskGraph);
	auto VisitNode = [this, &taskGraph, &traceMap](lemon::ListDigraph::Node node) {
		unsigned depth = 0;
		for (lemon::ListDigraph::InArcIt inArc(taskGraph, node); inArc != lemon::INVALID; ++inArc) {
			lemon::ListDigraph::Node source = taskGraph.source(inArc);
			depth = std::max(depth, traceMap[source].depth);
		}
		depth += 1;
		traceMap[node].depth = depth;		
	};
	std::vector<std::future<void>> sourceFutures;
	for (lemon::ListDigraph::Node source : sourceNodes) {
		std::future<void> fut = m_jobScheduler.EnqueueFuture(TraverseNode<decltype(VisitNode)>, 
															 std::ref(m_jobScheduler),
															 std::ref(taskGraph),
															 std::ref(dependencyCounter),
															 source,
															 VisitNode);
		sourceFutures.push_back(std::move(fut));
	}

	for (auto& fut : sourceFutures) {
		fut.get();
	}

	// Print tracemap into dot file
	std::stringstream ss;
	ss << "digraph {" << std::endl
		<< "rankdir = LR;" << std::endl
		<< "ranksep = \"0.8\";" << std::endl
		<< "node[shape = circle];" << std::endl;
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		unsigned depth = traceMap[nodeIt].depth;

		float t = std::clamp(depth / 35.f, 0.0f, 1.0f);
		float red = t;
		float blue = 1.0f-t;
		float green = 0.5f-std::abs(t-0.5f);

		ss << "node" << lemon::ListDigraph::id(nodeIt) << " ";
		ss << "[shape=circle, " << "style=filled, ";
		ss << "label=" << depth << ", ";
		ss << "fillcolor=\"#";
		ss << std::setfill('0') << std::setw(2) << std::hex << int(255.f*red);
		ss << std::setfill('0') << std::setw(2) << std::hex << int(255.f*green);
		ss << std::setfill('0') << std::setw(2) << std::hex << int(255.f*blue);
		ss << "\"";
		ss << std::dec;
		ss << "];" << std::endl;
	}
	for (lemon::ListDigraph::ArcIt arcIt(taskGraph); arcIt != lemon::INVALID; ++arcIt) {
		lemon::ListDigraph::Node source = taskGraph.source(arcIt);
		lemon::ListDigraph::Node target = taskGraph.target(arcIt);
		ss << "node" << lemon::ListDigraph::id(source);
		ss << " -> ";
		ss << "node" << lemon::ListDigraph::id(target) << ";" << std::endl;
	}
	ss << "}";
	//std::ofstream file(R"(D:\Programming\Inline-Engine\Test\QC_Simulator\pipeline_parallel.dot)", std::ios::trunc);
	//file << ss.str();
	//file.close();
}


//------------------------------------------------------------------------------
// Single threaded rendering
//------------------------------------------------------------------------------

void Scheduler::ExecuteSerial(FrameContext context) {
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
				context.commandQueue->GetUnderlyingQueue()->BeginDebuggerEvent("Node - " + className);
				AtScopeExit endPass([&context] { context.commandQueue->GetUnderlyingQueue()->EndDebuggerEvent(); });
				renderContext.TMP_SetCommandListName("Node - " + className + " - Execute()");
				

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
						injectList->BeginDebuggerEvent("Node - " + className + " - Barrier Injection");

						injectList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
						injectList->EndDebuggerEvent();
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

					decomposition.commandList->EndDebuggerEvent();
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
			injectList->BeginDebuggerEvent("Backbuffer State");

			injectList->ResourceBarrier(gxapi::TransitionBarrier{
				context.backBuffer->GetResource()._GetResourcePtr(),
				context.backBuffer->GetResource().ReadState(0),
				gxapi::eResourceState::PRESENT });
			injectList->EndDebuggerEvent();
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


//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------

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



//------------------------------------------------------------------------------
// Failure handling
//------------------------------------------------------------------------------

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



//------------------------------------------------------------------------------
// Upload task
//------------------------------------------------------------------------------


void Scheduler::UploadTask::Setup(SetupContext& context) {
	return;
}
void Scheduler::UploadTask::Execute(RenderContext& context) {
	CopyCommandList& commandList = context.AsGraphics();

	for (auto& request : *m_uploads) {
		// Init copy parameters
		auto& source = request.source;
		auto& destination = request.destination;

		// Set resource states
		commandList.SetResourceState(destination, gxapi::eResourceState::COPY_DEST);

		auto destType = request.destType;

		if (destType == UploadManager::DestType::BUFFER) {
			auto& dstBuffer = static_cast<const LinearBuffer&>(destination);
			commandList.CopyBuffer(dstBuffer, request.dstOffsetX, source, 0, source.GetSize());
		}
		else if (destType == UploadManager::DestType::TEXTURE_2D) {
			auto& dstTexture = static_cast<const Texture2D&>(destination);
			SubTexture2D dstPlace(request.dstSubresource, Vector<intptr_t, 2>((intptr_t)request.dstOffsetX, (intptr_t)request.dstOffsetY));
			commandList.CopyTexture(dstTexture, source, dstPlace, request.textureBufferDesc);
		}
	}
}



} // namespace gxeng
} // namespace inl
