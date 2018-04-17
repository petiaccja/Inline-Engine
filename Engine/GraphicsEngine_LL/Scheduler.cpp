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

void Scheduler::WriteTraceFile(const lemon::ListDigraph& taskGraph, const ExecuteState& state) {
	std::string graphvizDotText = WriteTraceGraphviz(taskGraph, state.trace);
	std::experimental::filesystem::path traceFilePath = R"(D:\Programming\Inline-Engine\Test\QC_Simulator)";
	if (std::experimental::filesystem::exists(traceFilePath)) {
		std::ofstream file(traceFilePath / "pipeline_parallel.dot");
		file << graphvizDotText;
	}
}

void Scheduler::ExecuteParallel(FrameContext context) {
	try {
		// Get graphs.
		const auto& taskGraph = m_pipeline.GetTaskGraph();
		const auto& taskFunctions = m_pipeline.GetTaskFunctionMap();

		// Get dependency tracker.
		lemon::ListDigraph::NodeMap<TraverseDependency> dependencyTracker(taskGraph);
		GetTraverseDependencies(taskGraph, dependencyTracker);

		// Launch variables.
		VolatileViewHeap vheap(context.gxApi);
		ExecuteState state(taskGraph, vheap, context);
		std::vector<jobs::Future<void>> jobFutures;
		std::vector<lemon::ListDigraph::Node> sourceNodes = GetSourceNodes(taskGraph);
		m_finishedListRemNodes.store(lemon::countNodes(taskGraph));

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

		// Wait in execute tasks and execute incoming command lists.
		EnqueueFinishedLists(context, jobFutures);
		for (auto& future : jobFutures) {
			future.get();
		}

		// Print trace into a graphviz dot file.
		WriteTraceFile(taskGraph, state);
	}
	catch (Exception& ex) {
		std::cout << "=== This frame is fucked ===" << std::endl;
		std::cout << "Error message: " << std::endl << ex.what() << std::endl;

		std::cout << std::endl;
		Exception::BreakOnce();
	}
	catch (std::exception& ex) {
		std::cout << "This frame is doubly fucked: " << ex.what() << std::endl;
	}
}


void Scheduler::GetNodeNames(ExecuteState& state) const {
	const auto& depGraph = m_pipeline.GetDependencyGraph();
	const auto& nodeMap = m_pipeline.GetNodeMap();
	const auto& parentMap = m_pipeline.GetTaskParentMap();

	for (lemon::ListDigraph::NodeIt nodeIt(state.taskGraph); nodeIt != lemon::INVALID; ++nodeIt) {
		lemon::ListDigraph::Node parent = parentMap[nodeIt];
		const NodeBase* pipelineNode = nodeMap[parent].get();
		std::string className = typeid(*pipelineNode).name();

		size_t idx = className.find("inl::gxeng::");
		if (idx != className.npos) { 
			className = className.substr(idx+12); 
		}

		state.trace[nodeIt].nodeName = className;
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
	// Calculate depth for tracing.
	unsigned depth = 0;
	for (lemon::ListDigraph::InArcIt inArc(state.taskGraph, node); inArc != lemon::INVALID; ++inArc) {
		lemon::ListDigraph::Node source = state.taskGraph.source(inArc);
		depth = std::max(depth, state.trace[source].depth);
	}
	depth += 1;
	state.trace[node].depth = depth;

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


void Scheduler::ExecuteNode(lemon::ListDigraph::Node node, ExecuteState& state) {
	bool signaled = false;
	try {
		// Acquire task.
		GraphicsTask* task = m_pipeline.GetTaskFunctionMap()[node];
		if (task == nullptr) {
			return;
		}


		// See if we can inherit a command list.
		std::unique_ptr<BasicCommandList> inheritedCommandList;
		std::unique_ptr<VolatileViewHeap> inheritedVheap;
		if (lemon::countInArcs(state.taskGraph, node) == 1) {
			lemon::ListDigraph::Node prevNode = state.taskGraph.source(lemon::ListDigraph::InArcIt(state.taskGraph, node));
			inheritedCommandList = std::move(state.trace[prevNode].inheritableList);
			inheritedVheap = std::move(state.trace[prevNode].inheritableVheap);
		}

		// Record trace.
		bool couldInheritList = (bool)inheritedCommandList;

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

		// Handle produced command list(s).
		std::unique_ptr<BasicCommandList> currentCommandList;
		renderContext.Decompose(inheritedCommandList, currentCommandList);
		if (currentCommandList) {
			currentCommandList->EndDebuggerEvent();
		}
		m_finishedListRemNodes.fetch_sub(1);
		signaled = true;

		// Record trace.
		state.trace[node].inheritedCommandList = couldInheritList && !inheritedCommandList;
		state.trace[node].producedCommandList = (bool)currentCommandList;

		// Handle produced command list(s).
		if (inheritedCommandList) {
			FinishList(std::move(inheritedCommandList), std::move(inheritedVheap));
		}
		if (currentCommandList) {
			bool canNextNodeInherit = false;
			if (lemon::countOutArcs(state.taskGraph, node) == 1) {
				lemon::ListDigraph::OutArcIt theOnlyOutArc(state.taskGraph, node);
				lemon::ListDigraph::Node theOnlyNextNode = state.taskGraph.target(theOnlyOutArc);
				canNextNodeInherit = lemon::countInArcs(state.taskGraph, theOnlyNextNode) == 1;
			}
			if (canNextNodeInherit) {
				state.trace[node].inheritableList = std::move(currentCommandList);
				state.trace[node].inheritableVheap = std::move(vheap);
			}
			else {
				FinishList(std::move(currentCommandList), std::move(vheap));
			}
		}
	}
	catch (...) {
		if (!signaled) {
			m_finishedListRemNodes.fetch_sub(1);
			m_finishedListCv.notify_all();
		}
		throw;
	}
}


void Scheduler::FinishList(std::unique_ptr<BasicCommandList> list, std::unique_ptr<VolatileViewHeap> vheap) {
	std::lock_guard<SpinMutex> lkg(m_finishedListMtx);
	m_finishedLists.push({ std::move(list), std::move(vheap) });
	m_finishedListCv.notify_all();
}


void Scheduler::EnqueueFinishedLists(const FrameContext& context, const std::vector<jobs::Future<void>>& futures) {
	decltype(m_finishedLists) localLists;
	BasicCommandList::Decomposition previousList;
	std::unique_ptr<VolatileViewHeap> prevVheap;

	previousList.commandAllocator = context.commandAllocatorPool->RequestAllocator(gxapi::eCommandListType::GRAPHICS);
	previousList.commandList = context.commandListPool->RequestGraphicsList(previousList.commandAllocator.get());
	

	// Initialize previous list with the upload task and a dummy barrier-only list.
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


	bool jobsReady = false;
	auto JobsReady = [&futures] {
		bool jobsReady = true;
		for (auto& future : futures) {
			jobsReady = jobsReady && future.ready();
		}
		return jobsReady;
	};
	do {
		jobsReady = JobsReady();

		// Acquire pending command lists.
		std::unique_lock<SpinMutex> lk(m_finishedListMtx);
		m_finishedListCv.wait(lk, [this] { return !m_finishedLists.empty() || m_finishedListRemNodes == 0; });
		while (!m_finishedLists.empty()) {
			localLists.push(std::move(m_finishedLists.front()));
			m_finishedLists.pop();
		}
		lk.unlock();

		// Process pending command lists.
		while (!localLists.empty()) {
			auto* prevCopyList = dynamic_cast<gxapi::ICopyCommandList*>(previousList.commandList.get());
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
			usedResourceList.reserve(previousList.usedResources.size() + previousList.additionalResources.size());
			for (auto& v : previousList.usedResources) {
				usedResourceList.push_back(std::move(v.resource));
			}
			for (auto& v : previousList.additionalResources) {
				usedResourceList.push_back(std::move(v));
			}

			// Finally close and enqueue previous with the barriers added.
			prevCopyList->Close();

			EnqueueCommandList(*context.commandQueue,
							   std::move(previousList.commandList),
							   std::move(previousList.commandAllocator),
							   std::move(previousList.scratchSpaces),
							   std::move(usedResourceList),
							   std::move(prevVheap),
							   context);

			// Store current list so that it can take the barriers of the next one, if any.
			previousList = std::move(currentList);
			prevVheap = std::move(currentVheap);
			localLists.pop();
		}

	} while (!jobsReady);

	// Set backBuffer to PRESENT state.
	gxapi::eResourceState bbState = context.backBuffer->GetResource().ReadState(0);
	auto* prevCopyList = dynamic_cast<gxapi::ICopyCommandList*>(previousList.commandList.get());
	if (bbState != gxapi::eResourceState::PRESENT) {
		gxapi::TransitionBarrier barrier(context.backBuffer->GetResource()._GetResourcePtr(), bbState, gxapi::eResourceState::PRESENT);
		prevCopyList->ResourceBarrier(barrier);
	}
	prevCopyList->Close();

	// Enqueue the last command list.
	// Collect used resource of previous list.
	std::vector<MemoryObject> usedResourceList;
	usedResourceList.reserve(previousList.usedResources.size() + previousList.additionalResources.size());
	for (auto& v : previousList.usedResources) {
		usedResourceList.push_back(std::move(v.resource));
	}
	for (auto& v : previousList.additionalResources) {
		usedResourceList.push_back(std::move(v));
	}
	EnqueueCommandList(*context.commandQueue,
					   std::move(previousList.commandList),
					   std::move(previousList.commandAllocator),
					   std::move(previousList.scratchSpaces),
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
				renderContext.SetCommandListName("Node - " + className + " - Execute()");
				

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
