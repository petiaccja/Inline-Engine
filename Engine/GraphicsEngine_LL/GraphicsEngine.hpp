#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"
#include "ScratchSpacePool.hpp"
#include "ResourceResidencyQueue.hpp"
#include "PipelineEventDispatcher.hpp"
#include "PipelineEventListener.hpp"

#include "CriticalBufferHeap.hpp"
#include "BackBufferManager.hpp"
#include "MemoryManager.hpp"
#include "HostDescHeap.hpp"
#include "ShaderManager.hpp"

#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>

#include <BaseLibrary/Logging_All.hpp>


namespace inl {
namespace gxeng {


class Mesh;
class Image;
class Material;
class MaterialShaderEquation;
class MaterialShaderGraph;

class Scene;
class MeshEntity;
class Overlay;
class OverlayEntity;
class PerspectiveCamera;
class OrthographicCamera;

class WindowResizeListener;


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
	Image* CreateImage();
	Material* CreateMaterial();
	MaterialShaderEquation* CreateMaterialShaderEquation();
	MaterialShaderGraph* CreateMaterialShaderGraph();

	// Scene
	Scene* CreateScene(std::string name);
	Overlay* CreateOverlay(std::string name);
	MeshEntity* CreateMeshEntity();
	OverlayEntity* CreateOverlayEntity();
	PerspectiveCamera* CreatePerspectiveCamera(std::string name);
	OrthographicCamera* CreateOrthographicCamera(std::string name);
private:
	void CreatePipeline();
private:
	// Graphics API things
	gxapi::IGxapiManager* m_gxapiManager; // external resource, we should not delete it
	gxapi::IGraphicsApi* m_graphicsApi; // external resource, we should not delete it
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// Memory
	// NOTE: Pipeline is constructed after (and destructed before) view heaps because
	// pipeline nodes use these heaps through the graphics context.
	MemoryManager m_memoryManager;
	// FIXME: these heaps might not belong to the graphics engine instance
	DSVHeap m_dsvHeap;
	RTVHeap m_rtvHeap;
	CbvSrvUavHeap m_persResViewHeap;
	std::unique_ptr<BackBufferManager> m_backBufferHeap;
	std::vector<WindowResizeListener*> m_windowResizeListeners;

	// Pipeline Facilities
	CommandAllocatorPool m_commandAllocatorPool;
	ScratchSpacePool m_scratchSpacePool; // Creates CBV_SRV_UAV type scratch spaces
	CbvSrvUavHeap m_textureSpace;
	Pipeline m_pipeline;
	Scheduler m_scheduler;
	ShaderManager m_shaderManager;
	std::vector<SyncPoint> m_frameEndFenceValues;
	std::vector<GraphicsNode*> m_graphicsNodes;

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
	std::set<Overlay*> m_overlays;
	std::set<BasicCamera*> m_cameras;

private:
	void InitializeGraphicsNodes();
};



} // namespace gxeng
} // namespace inl