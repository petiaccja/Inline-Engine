#include "Scheduler.hpp"

#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <BaseLibrary/AtScopeExit.hpp>

#include "GraphicsCommandList.hpp"

#include <cassert>
#include <typeinfo> // Track render passes.
#include <fstream> // only for debugging
#include <sstream>
#include <iomanip>
#include <atomic>

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


void Scheduler::Execute(FrameContext context) {
	try {
		// Get graphs.
		const auto& taskGraph = m_pipeline.GetTaskGraph();

		// Get dependency tracker.
		lemon::ListDigraph::NodeMap<TraverseDependency> dependencyTracker(taskGraph);
		GetTraverseDependencies(taskGraph, dependencyTracker);

		// Launch variables.
		ExecuteState state(taskGraph, context);
		std::vector<jobs::Future<void>> jobFutures;
		std::vector<lemon::ListDigraph::Node> sourceNodes = GetSourceNodes(taskGraph);

		GetNodeNames(state);

		// Launch setup tasks.
		auto SetupNodeWrapper = [this, &state](lemon::ListDigraph::Node node) {
			SetupNode(node, state);
		};
		for (auto sourceNode : sourceNodes) {
			jobs::Future<void> future = m_jobScheduler.Enqueue(TraverseNode<decltype(SetupNodeWrapper)>,
															   sourceNode,
															   std::ref(taskGraph),
															   std::ref(m_jobScheduler),
															   std::ref(dependencyTracker),
															   SetupNodeWrapper);
			jobFutures.push_back(std::move(future));
		}

		// Wait in setup tasks.
		for (auto& future : jobFutures) {
			future.get();
		}

		// Launch execute tasks.
		jobFutures.clear();
		for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
			dependencyTracker[nodeIt].ResetCounter();
		}
		auto ExecuteNodeWrapper = [this, &state](lemon::ListDigraph::Node node) {
			ExecuteNode(node, state);
		};
		for (auto sourceNode : sourceNodes) {
			jobs::Future<void> future = m_jobScheduler.Enqueue(TraverseNode<decltype(ExecuteNodeWrapper)>,
															   sourceNode,
															   std::ref(taskGraph),
															   std::ref(m_jobScheduler),
															   std::ref(dependencyTracker),
															   ExecuteNodeWrapper);
			jobFutures.push_back(std::move(future));
		}

		m_finishedListDone = false;
		auto OnEnd = [&]() -> jobs::Future<void> {
			AtScopeExit makeSignal([&]{
				std::unique_lock<SpinMutex> lkg(m_finishedListMtx);
				m_finishedListDone = true;
				lkg.unlock();
				m_finishedListCv.notify_one();				
			});

			// May throw.
			for (auto& future : jobFutures) {
				co_await future;
			}		
		};
		auto onEndFut = OnEnd();
		onEndFut.Run();

		// Wait in execute tasks and execute incoming command lists.
		EnqueueFinishedLists(context, jobFutures);
		onEndFut.get();
	}
	catch (Exception& ex) {
		std::cout << "=== This frame is fucked ===" << std::endl;
		std::cout << "Error message: " << std::endl << ex.what() << std::endl;

		std::cout << std::endl;
		Exception::BreakOnce();
	}
	catch (std::exception& ex) {
		std::cout << "=== This frame is fucked for unknown reasons ===" << std::endl;
		std::cout << "Error message:" << std::endl << ex.what() << std::endl;
	}
}


void Scheduler::GetNodeNames(ExecuteState& state) const {
	const auto& nodeMap = m_pipeline.GetNodeMap();
	const auto& parentMap = m_pipeline.GetTaskParentMap();

	for (lemon::ListDigraph::NodeIt nodeIt(state.taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		lemon::ListDigraph::Node parent = parentMap[nodeIt];
		const NodeBase* pipelineNode = nodeMap[parent].get();
		state.trace[nodeIt].nodeName = pipelineNode->GetClassName(true, { "inl", "gxeng" });
	}
}


void Scheduler::GetTraverseDependencies(const lemon::ListDigraph& graph, lemon::ListDigraph::NodeMap<TraverseDependency>& dependencyTracker) {
	for (lemon::ListDigraph::NodeIt nodeIt(graph); nodeIt != lemon::INVALID; ++nodeIt) {
		size_t numDependencies = countInArcs(graph, nodeIt);
		dependencyTracker[nodeIt].ResetCounter();
		dependencyTracker[nodeIt].Set(numDependencies);
	}
}


std::vector<lemon::ListDigraph::Node> Scheduler::GetSourceNodes(const lemon::ListDigraph& graph) {
	std::vector<lemon::ListDigraph::Node> sources;

	for (lemon::ListDigraph::NodeIt nodeIt(graph); nodeIt != lemon::INVALID; ++nodeIt) {
		size_t numDependencies = countInArcs(graph, nodeIt);
		if (numDependencies == 0) {
			sources.push_back(nodeIt);
		}
	}

	return sources;
}


// Can be in the .cpp because it is only used in this compilation unit.
template <class Func>
jobs::Future<void> Scheduler::TraverseNode(lemon::ListDigraph::Node node,
										   const lemon::ListDigraph& graph,
										   jobs::Scheduler& scheduler,
										   lemon::ListDigraph::NodeMap<TraverseDependency>& dependencyTracker,
										   Func onVisit)
{
	onVisit(node);

	std::vector<jobs::Future<void>> childJobs;
	for (lemon::ListDigraph::OutArcIt outArc(graph, node); outArc != lemon::INVALID; ++outArc) {
		auto nextNode = graph.target(outArc);

		if (++dependencyTracker[nextNode]) {
			auto nextTask = scheduler.Enqueue(TraverseNode<Func>,
											  nextNode,
											  std::ref(graph),
											  std::ref(scheduler),
											  std::ref(dependencyTracker),
											  onVisit);
			childJobs.push_back(std::move(nextTask));
		}
	}

	for (auto& childFuture : childJobs) {
		co_await childFuture;
	}
}



void Scheduler::SetupNode(lemon::ListDigraph::Node node, ExecuteState& state) {
	// Acquire task.
	GraphicsTask* task = m_pipeline.GetTaskFunctionMap()[node];
	if (task == nullptr) {
		return;
	}

	// Setup given node.
	SetupContext setupContext(state.frameContext.memoryManager,
								state.frameContext.textureSpace,
								state.frameContext.rtvHeap,
								state.frameContext.dsvHeap,
								state.frameContext.shaderManager,
								state.frameContext.gxApi);
	task->Setup(setupContext);	
}


std::pair<std::unique_ptr<BasicCommandList>, std::unique_ptr<VolatileViewHeap>> Scheduler::InheritCommandList(lemon::ListDigraph::Node node, ExecuteState& state) {
	if (lemon::countInArcs(state.taskGraph, node) == 1) {
		lemon::ListDigraph::Node prevNode = state.taskGraph.source(lemon::ListDigraph::InArcIt(state.taskGraph, node));
		std::pair<std::unique_ptr<BasicCommandList>, std::unique_ptr<VolatileViewHeap>> ret{
			std::move(state.trace[prevNode].inheritableList),
			std::move(state.trace[prevNode].inheritableVheap)
		};
		return ret;
	}
	return { nullptr, nullptr };
}

bool Scheduler::CanNextInheritCommandList(lemon::ListDigraph::Node node, ExecuteState& state) {
	if (lemon::countOutArcs(state.taskGraph, node) == 1) {
		lemon::ListDigraph::OutArcIt theOnlyOutArc(state.taskGraph, node);
		lemon::ListDigraph::Node theOnlyNextNode = state.taskGraph.target(theOnlyOutArc);
		return lemon::countInArcs(state.taskGraph, theOnlyNextNode) == 1;
	}
	return false;
}


void Scheduler::ExecuteNode(lemon::ListDigraph::Node node, ExecuteState& state) {
	// When node is done, we want to notify the queue collector about it. (Scheduler on main thread.)
	AtScopeExit notifyOnExit([&, this] {
		m_finishedListCv.notify_all();
	});

	// Acquire task.
	GraphicsTask* task = m_pipeline.GetTaskFunctionMap()[node];
	if (task == nullptr) {
		return;
	}

	// See if we can inherit a command list.
	auto[inheritedCommandList, inheritedVheap] = InheritCommandList(node, state);

	// Execute given node.
	auto vheap = std::make_unique<VolatileViewHeap>(state.frameContext.gxApi);
	RenderContext renderContext(state.frameContext.memoryManager,
								state.frameContext.textureSpace,
								vheap.get(),
								state.frameContext.shaderManager,
								state.frameContext.gxApi,
								state.frameContext.commandListPool,
								state.frameContext.commandAllocatorPool,
								state.frameContext.scratchSpacePool,
								std::move(inheritedCommandList));
	renderContext.SetCommandListName(state.trace[node].nodeName);
	task->Execute(renderContext);

	// Get inherited and current list, if any.
	std::unique_ptr<BasicCommandList> currentCommandList;
	renderContext.Decompose(inheritedCommandList, currentCommandList);


	// Schedule inherited command list.
	if (inheritedCommandList) {
		FinishList(std::move(inheritedCommandList), std::move(inheritedVheap));
	}

	// Scheduler or pass on current command list.
	if (currentCommandList) {
		currentCommandList->EndDebuggerEvent(); // End marker for profilers.

		if (CanNextInheritCommandList(node, state)) {
			state.trace[node].inheritableList = std::move(currentCommandList);
			state.trace[node].inheritableVheap = std::move(vheap);
		}
		else {
			FinishList(std::move(currentCommandList), std::move(vheap));
		}
	}
}


void Scheduler::FinishList(std::unique_ptr<BasicCommandList> list, std::unique_ptr<VolatileViewHeap> vheap) {
	std::lock_guard<SpinMutex> lkg(m_finishedListMtx);
	m_finishedLists.push({ std::move(list), std::move(vheap) });
}


void Scheduler::EnqueueFinishedLists(const FrameContext& context, const std::vector<jobs::Future<void>>& futures) {
	decltype(m_finishedLists) localLists;
	BasicCommandList::Decomposition prevList;
	std::unique_ptr<VolatileViewHeap> prevVheap;

	// Initialize previous list with the upload task and a dummy barrier-only list.
	prevList.commandAllocator = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
	prevList.commandList = context.commandListPool->RequestGraphicsList(prevList.commandAllocator.get());
	

	UploadTask uploadTask(context.uploadRequests);
	SetupContext setupContext(context.memoryManager,
							  context.textureSpace,
							  context.rtvHeap,
							  context.dsvHeap,
							  context.shaderManager,
							  context.gxApi);
	auto uploadVheap= std::make_unique<VolatileViewHeap>(context.gxApi);
	RenderContext renderContext(context.memoryManager,
								context.textureSpace,
								prevVheap.get(),
								context.shaderManager,
								context.gxApi,
								context.commandListPool,
								context.commandAllocatorPool,
								context.scratchSpacePool);
	uploadTask.Setup(setupContext);
	uploadTask.Execute(renderContext);
	std::unique_ptr<BasicCommandList> uploadInherit, uploadList;
	renderContext.Decompose(uploadInherit, uploadList);
	localLists.push({ std::move(uploadList), std::move(uploadVheap) });


	do {
		// Acquire pending command lists.
		std::unique_lock<SpinMutex> lk(m_finishedListMtx);
		m_finishedListCv.wait(lk, [this] { return !m_finishedLists.empty() || m_finishedListDone; });
		while (!m_finishedLists.empty()) {
			localLists.push(std::move(m_finishedLists.front()));
			m_finishedLists.pop();
		}
		lk.unlock();

		// Process pending command lists.
		while (!localLists.empty()) {
			auto* prevCopyList = dynamic_cast<gxapi::ICopyCommandList*>(prevList.commandList.get());
			auto currentList = std::get<0>(localLists.front())->Decompose();
			auto currentVheap = std::move(std::get<1>(localLists.front()));

			// Add current list's barriers to previous list.
			std::vector<gxapi::ResourceBarrier> barriers = InjectBarriers(currentList.usedResources.begin(), currentList.usedResources.end());
			if (!barriers.empty()) {
				prevCopyList->ResourceBarrier((unsigned)barriers.size(), barriers.data());
			}

			// Update resource states to reflect current list.
			UpdateResourceStates(currentList.usedResources.begin(), currentList.usedResources.end());
			
			// Collect used resource of previous list.
			std::vector<MemoryObject> usedResourceList;
			usedResourceList.reserve(prevList.usedResources.size() + prevList.additionalResources.size());
			for (auto& v : prevList.usedResources) {
				usedResourceList.push_back(std::move(v.resource));
			}
			for (auto& v : prevList.additionalResources) {
				usedResourceList.push_back(std::move(v));
			}

			// Finally close and enqueue previous with the barriers added.
			prevCopyList->Close();

			EnqueueCommandList(*context.commandQueue,
							   std::move(prevList.commandList),
							   std::move(prevList.commandAllocator),
							   std::move(prevList.scratchSpaces),
							   std::move(usedResourceList),
							   std::move(prevVheap),
							   context);

			// Store current list so that it can take the barriers of the next one, if any.
			prevList = std::move(currentList);
			prevVheap = std::move(currentVheap);
			localLists.pop();
		}

	} while (!m_finishedListDone);

	// Set backBuffer to PRESENT state.
	gxapi::eResourceState bbState = context.backBuffer->GetResource().ReadState(0);
	auto* prevCopyList = dynamic_cast<gxapi::ICopyCommandList*>(prevList.commandList.get());
	if (bbState != gxapi::eResourceState::PRESENT) {
		gxapi::TransitionBarrier barrier(context.backBuffer->GetResource()._GetResourcePtr(), bbState, gxapi::eResourceState::PRESENT);
		prevCopyList->ResourceBarrier(barrier);
	}
	prevCopyList->Close();

	// Enqueue the last command list.
	// Collect used resource of previous list.
	std::vector<MemoryObject> usedResourceList;
	usedResourceList.reserve(prevList.usedResources.size() + prevList.additionalResources.size());
	for (auto& v : prevList.usedResources) {
		usedResourceList.push_back(std::move(v.resource));
	}
	for (auto& v : prevList.additionalResources) {
		usedResourceList.push_back(std::move(v));
	}
	EnqueueCommandList(*context.commandQueue,
					   std::move(prevList.commandList),
					   std::move(prevList.commandAllocator),
					   std::move(prevList.scratchSpaces),
					   std::move(usedResourceList),
					   std::move(prevVheap),
					   context);
}


std::string Scheduler::WriteTraceGraphviz(const lemon::ListDigraph& taskGraph, const lemon::ListDigraph::NodeMap<ExecuteTrace>& traceMap) {
	std::stringstream ss;
	ss << "digraph {" << std::endl
		<< "rankdir = LR;" << std::endl
		<< "ranksep = \"0.8\";" << std::endl
		<< "node[shape = circle];" << std::endl;
	for (lemon::ListDigraph::NodeIt nodeIt(taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		unsigned depth = traceMap[nodeIt].depth;
		bool hasListInside = (bool)traceMap[nodeIt].inheritableList;

		std::string labelStatus;
		labelStatus += (hasListInside ? "ACTIVE" : "EMPTY");
		labelStatus += (traceMap[nodeIt].inheritedCommandList ? "_I" : "");
		labelStatus += (traceMap[nodeIt].producedCommandList ? "_P" : "");

		float t = std::clamp(depth / 35.f, 0.0f, 1.0f);
		float red = t;
		float blue = 1.0f-t;
		float green = 0.5f-std::abs(t-0.5f);

		ss << "node" << lemon::ListDigraph::id(nodeIt) << " ";
		ss << "[shape=circle, " << "style=filled, ";
		ss << "label=" << "\"" << depth << "_" << labelStatus << "\", ";
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
	return ss.str();
}


void Scheduler::WriteTraceFile(const lemon::ListDigraph& taskGraph, const ExecuteState& state) {
	std::string graphvizDotText = WriteTraceGraphviz(taskGraph, state.trace);
	std::experimental::filesystem::path traceFilePath = R"(D:\Programming\Inline-Engine\Test\QC_Simulator)";
	if (std::experimental::filesystem::exists(traceFilePath)) {
		std::ofstream file(traceFilePath / "pipeline_parallel.dot");
		file << graphvizDotText;
	}
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
