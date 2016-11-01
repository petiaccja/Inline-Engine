#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"
#include "ResourceResidencyQueue.hpp"
#include "PipelineEventDispatcher.hpp"
#include "PipelineEventListener.hpp"

#include "ResourceHeap.hpp"
#include "MemoryManager.hpp"
#include "HighLevelDescHeap.hpp"

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>

#include <BaseLibrary/Logging_All.hpp>

// DELETE THIS
#include "Nodes/Node_GetDepthBuffer.hpp"


namespace inl {
namespace gxeng {


class Mesh;
class Image;

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


// Temporary, delete this!!!!44négy
class PipelineEventPrinter : public PipelineEventListener {
public:
	PipelineEventPrinter() : m_log(nullptr) {}

	void SetLog(exc::LogStream* log) { m_log = log; }

	void OnFrameBeginDevice(uint64_t frameId) override {
		m_log->Event(exc::Event{ "Frame begin - DEVICE", exc::EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameBeginHost(uint64_t frameId) override {
		m_log->Event(exc::Event{ "Frame begin - HOST", exc::EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameCompleteDevice(uint64_t frameId) override {
		m_log->Event(exc::Event{ "Frame finished - DEVICE", exc::EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameCompleteHost(uint64_t frameId) override {
		m_log->Event(exc::Event{ "Frame finished - HOST", exc::EventParameterInt("frameId", (int)frameId) });
	}
private:
	exc::LogStream* m_log;
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
	void SetScreenSize(unsigned width, unsigned height);
	void GetScreenSize(unsigned& width, unsigned& height);
	void SetFullScreen(bool enable);
	bool GetFullScreen() const;

	// Resources
	Mesh* CreateMesh();

	// Scene
	Scene* CreateScene(std::string name);
	MeshEntity* CreateMeshEntity();
	Texture2DSRV DEBUG_CreateTexture(const void* data, uint32_t width, uint32_t height, gxapi::eFormat format);
	Image* CreateImage();
private:
	void CreatePipeline();
private:
	// Graphics API things
	gxapi::IGxapiManager* m_gxapiManager; // external resource, we should not delete it
	gxapi::IGraphicsApi* m_graphicsApi; // external resource, we should not delete it
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// Pipeline Facilities
	CommandAllocatorPool m_commandAllocatorPool;
	ScratchSpacePool m_scratchSpacePool; // Creates CBV_SRV_UAV type scratch spaces
	PersistentResViewHeap m_DEBUG_textureSpace;
	Pipeline m_pipeline;
	Scheduler m_scheduler;
	std::vector<SyncPoint> m_frameEndFenceValues;

	// Memory
	MemoryManager m_memoryManager;
	DSVHeap m_dsvHeap; // TODO
	std::unique_ptr<BackBufferHeap> m_backBufferHeap;

	// Pipeline elements
	CommandQueue m_masterCommandQueue;
	ResourceResidencyQueue m_residencyQueue;
	PipelineEventDispatcher m_pipelineEventDispatcher;
	PipelineEventPrinter m_pipelineEventPrinter; // DELETE THIS

	// Logging
	exc::Logger* m_logger;
	exc::LogStream m_logStreamGeneral;
	exc::LogStream m_logStreamPipeline;

	// Misc
	std::chrono::nanoseconds m_absoluteTime;
	uint64_t m_frame = 0;

	// Scene
	std::set<Scene*> m_scenes;

	// DELETE THIS
	nodes::GetDepthBuffer* m_getBBNode;
};



} // namespace gxeng
} // namespace inl