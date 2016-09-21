#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"
#include "CommandListTasks.hpp"
#include "ResourceResidencyQueue.hpp"

#include "ResourceHeap.hpp"

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>

#include <BaseLibrary/Logging_All.hpp>


namespace inl {
namespace gxeng {


class TypedMesh;
class Scene;
class MeshEntity;


struct GraphicsEngineDesc {
	gxapi::IGxapiManager* gxapiManager;
	gxapi::IGraphicsApi* graphicsApi;
	gxapi::NativeWindowHandle targetWindow;
	bool fullScreen;
	int width;
	int height;
	exc::Logger* logger;
};


class GraphicsEngine {
public:
	// Custructors
	GraphicsEngine(GraphicsEngineDesc desc);
	GraphicsEngine(const GraphicsEngine&) = delete;
	GraphicsEngine& operator=(const GraphicsEngine&) = delete;
	~GraphicsEngine();

	// Update scene
	void Update(float elapsed);

	// Resources
	TypedMesh* CreateMesh();

	// Scene
	Scene* CreateScene(std::string name);
	MeshEntity* CreateMeshEntity();

private:
	void CreatePipeline();
private:
	// Graphics API things
	gxapi::IGxapiManager* m_gxapiManager; // external resource, we should not delete it
	gxapi::IGraphicsApi* m_graphicsApi; // external resource, we should not delete it
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// Pipeline Facilities
	CommandAllocatorPool m_commandAllocatorPool;
	ScratchSpacePool m_scratchSpacePool;
	std::mutex m_commandAllocatorMutex;
	Scheduler m_scheduler;
	Pipeline m_pipeline;
	std::vector<SyncPoint> m_frameEndFenceValues;

	// Pipeline elements
	CommandQueue m_masterCommandQueue;
	ResourceResidencyQueue m_residencyQueue;

	// Memory
	std::unique_ptr<BackBufferHeap> m_backBufferHeap;

	// Logging
	exc::Logger* m_logger;
	exc::LogStream m_logStreamGeneral;
	exc::LogStream m_logStreamPipeline;

	// Misc
	std::chrono::nanoseconds m_absoluteTime;
	uint64_t m_frame = 0;

	// Scene
	std::set<Scene*> m_scenes;
};



} // namespace gxeng
} // namespace inl