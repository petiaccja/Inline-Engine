#pragma once

#include "Pipeline.hpp"
#include "Scheduler.hpp"
#include "CommandAllocatorPool.hpp"
#include "CommandListPool.hpp"
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

#include <BaseLibrary/Any.hpp>


namespace inl {
namespace gxeng {


class Mesh;
class Image;
class Material;
class MaterialShaderEquation;
class MaterialShaderGraph;

class Scene;
class MeshEntity;
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
	Logger* logger;
};


// Temporary, delete this!!!!44négy
// Peti wuz here '17.07.28 - surprisingly, I actually understood why this is temporary, but I'm not gonna write it down
class PipelineEventPrinter : public PipelineEventListener {
public:
	PipelineEventPrinter() : m_log(nullptr) {}

	void SetLog(LogStream* log) { m_log = log; }

	void OnFrameBeginDevice(uint64_t frameId) override {
		m_log->Event(LogEvent{ "Frame begin - DEVICE", EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameBeginHost(uint64_t frameId) override {
		m_log->Event(LogEvent{ "Frame begin - HOST", EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameBeginAwait(uint64_t frameId) override {
		m_log->Event(LogEvent{ "Awaiting frame", EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameCompleteDevice(uint64_t frameId) override {
		m_log->Event(LogEvent{ "Frame finished - DEVICE", EventParameterInt("frameId", (int)frameId) });
	}
	void OnFrameCompleteHost(uint64_t frameId) override {
		m_log->Event(LogEvent{ "Frame finished - HOST", EventParameterInt("frameId", (int)frameId) });
	}
private:
	LogStream* m_log;
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
	MeshEntity* CreateMeshEntity();
	OverlayEntity* CreateOverlayEntity();
	PerspectiveCamera* CreatePerspectiveCamera(std::string name);
	OrthographicCamera* CreateOrthographicCamera(std::string name);

	// Environment variables
	/// <returns> True if a new variable was created, false if old was overridden. </returns>
	bool SetEnvVariable(std::string name, Any obj);
	bool EnvVariableExists(const std::string& name);
	const Any& GetEnvVariable(const std::string& name);
private:
	void CreatePipeline();
	static std::vector<GraphicsNode*> SelectSpecialNodes(Pipeline& pipeline);
	void UpdateSpecialNodes();
	static void DumpPipelineGraph(const Pipeline& pipeline, std::string file);
private:
	// Graphics API things
	gxapi::IGxapiManager* m_gxapiManager; // external resource, we should not delete it
	gxapi::IGraphicsApi* m_graphicsApi; // external resource, we should not delete it
	std::unique_ptr<gxapi::ISwapChain> m_swapChain;

	// Memory
	MemoryManager m_memoryManager;
	DSVHeap m_dsvHeap;
	RTVHeap m_rtvHeap;
	CbvSrvUavHeap m_persResViewHeap;
	std::unique_ptr<BackBufferManager> m_backBufferHeap;
	std::vector<WindowResizeListener*> m_windowResizeListeners;

	// Pipeline Facilities
	CommandAllocatorPool m_commandAllocatorPool;
	CommandListPool m_commandListPool;
	ScratchSpacePool m_scratchSpacePool; // Creates CBV_SRV_UAV type scratch spaces
	CbvSrvUavHeap m_textureSpace;
	Pipeline m_pipeline;
	Scheduler m_scheduler;
	ShaderManager m_shaderManager;
	std::vector<SyncPoint> m_frameEndFenceValues;
	std::vector<std::shared_ptr<GraphicsNode>> m_graphicsNodes;
	std::vector<GraphicsNode*> m_specialNodes;

	// Pipeline elements
	CommandQueue m_masterCommandQueue;
	ResourceResidencyQueue m_residencyQueue;
	PipelineEventDispatcher m_pipelineEventDispatcher;
	PipelineEventPrinter m_pipelineEventPrinter; // ONLY FOR TEST PURPOSES

	// Logging
	Logger* m_logger;
	LogStream m_logStreamGeneral;
	LogStream m_logStreamPipeline;

	// Misc
	std::chrono::nanoseconds m_absoluteTime;
	uint64_t m_frame = 0;

	// Env variables
	std::unordered_map<std::string, Any> m_envVariables;

	// Scene
	std::set<Scene*> m_scenes;
	std::set<BasicCamera*> m_cameras;
};



} // namespace gxeng
} // namespace inl