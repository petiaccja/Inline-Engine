#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"

#include "../GraphicsApi_LL/IGxapiManager.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/ISwapChain.hpp"


namespace inl {
namespace gxeng {


struct GraphicsEngineDesc {
	std::unique_ptr<gxapi::IGxapiManager> gxapiManager;
	std::unique_ptr<gxapi::IGraphicsApi> graphicsApi;
	gxapi::NativeWindowHandle targetWindow;
	bool fullScreen;
	int width;
	int height;
};


class GraphicsEngine {
public:
	GraphicsEngine(GraphicsEngineDesc desc);


	void Update(float elapsed);

private:
	// Graphics API things
	std::unique_ptr<gxapi::IGxapiManager> m_gxapiManager;
	std::unique_ptr<gxapi::IGraphicsApi> m_graphicsApi;
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// API Pipeline
	std::unique_ptr<gxapi::ICommandQueue> m_mainCommandQueue;

	// Facilities
	Scheduler m_scheduler;
	Pipeline m_pipeline;
	CommandAllocatorPool m_commandAllocatorPool;
};



} // namespace gxeng
} // namespace inl