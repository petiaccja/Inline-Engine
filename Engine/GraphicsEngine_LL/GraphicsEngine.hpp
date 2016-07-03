#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"
#include "CommandListTasks.hpp"

#include "../GraphicsApi_LL/IGxapiManager.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/ISwapChain.hpp"
#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include <queue>
#include <condition_variable>
#include <mutex>


namespace inl {
namespace gxeng {


struct GraphicsEngineDesc {
	gxapi::IGxapiManager* gxapiManager;
	gxapi::IGraphicsApi* graphicsApi;
	gxapi::NativeWindowHandle targetWindow;
	bool fullScreen;
	int width;
	int height;
};


class GraphicsEngine {
public:
	GraphicsEngine(GraphicsEngineDesc desc);
	GraphicsEngine(const GraphicsEngine&) = delete;
	GraphicsEngine& operator=(const GraphicsEngine&) = delete;
	~GraphicsEngine();

	void Update(float elapsed);
private:
	void CreatePipeline();

	void InitTaskThreadFunc();
	void CleanTaskThreadFunc();
private:
	// Graphics API things
	gxapi::IGxapiManager* m_gxapiManager; // external resource, we should not delete it
	gxapi::IGraphicsApi* m_graphicsApi; // external resource, we should not delete it
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// Pipeline elements
	CommandQueue m_masterCommandQueue;
	std::queue<InitTask>  m_initTasks;
	std::queue<CleanTask>  m_cleanTasks;
	std::thread m_initTaskThread;
	std::thread m_cleanTaskThread;
	volatile bool m_runInitCleanTasks;
	std::mutex m_initMutex; // replace w/ proper concurrent queue
	std::mutex m_cleanMutex; // replace w/ proper concurrent queue
	std::condition_variable m_initCv;
	std::condition_variable m_cleanCv;

	// Facilities
	Scheduler m_scheduler;
	Pipeline m_pipeline;
	CommandAllocatorPool m_commandAllocatorPool;
	std::mutex m_commandAllocatorMutex;
};



} // namespace gxeng
} // namespace inl