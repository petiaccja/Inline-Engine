#pragma once

#include "GraphicsNodeFactory.hpp"
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
#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>

#include <filesystem>


#undef CreateFont // Fuck goddamn winapi -.-


namespace inl {
namespace gxeng {


class Mesh;
class Image;
class Material;
class MaterialShaderEquation;
class MaterialShaderGraph;
class Font;

class Scene;
class MeshEntity;
class OverlayEntity;
class TextEntity;
class PerspectiveCamera;
class OrthographicCamera;
class Camera2D;


class WindowResizeListener;


struct GraphicsEngineDesc {
	gxapi::IGxapiManager* gxapiManager = nullptr;
	gxapi::IGraphicsApi* graphicsApi = nullptr;
	gxapi::NativeWindowHandle targetWindow = {};
	bool fullScreen = false;
	int width = 640;
	int height = 480;
	Logger* logger = nullptr;
};


// Temporary, delete this!!!!44négy
// Peti wuz here '17.07.28 - surprisingly, I actually understood why this is temporary, but I'm not gonna write it down
// Yeah, I still understand and this is really not important '17.11.07
// Okay, I know what it does, but why would I need it? Probably junk... '18.02.24
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

	/// <summary> Redraws the entire screen. </summary>
	/// <param name="elapsed"> Time since last frame in seconds. </param>
	/// <remarks>
	/// Since this call operates on all the entities and resources that compose the scene,
	/// you are not allowed to concurrently modify these objects while Update() is running.
	/// This includes but is not limited to adding new entities to a scene and uploading
	/// data to meshes or images.
	/// </remarks>
	void Update(float elapsed);

	/// <summary> Rescales the backbuffer. </summary>
	/// <remarks> Causes a pipeline flush, high overhead. </remarks>
	void SetScreenSize(unsigned width, unsigned height);

	/// <summary> Returns the current backbuffer size in the out parameters. </remarks>
	void GetScreenSize(unsigned& width, unsigned& height);

	/// <summary> Sets the D3D swap chain to full-screen mode. </summary>
	/// <param name="enable"> True to full screen, false to windowed. </param>
	void SetFullScreen(bool enable);

	/// <summary> True if the swap chain is currently in full-screen mode. </summary>
	bool GetFullScreen() const;


	// Graph editor interfaces
	IEditorGraph* QueryPipelineEditor() const;
	IEditorGraph* QueryMaterialEditor() const;


	// Resources
	Mesh* CreateMesh();
	Image* CreateImage();
	Material* CreateMaterial();
	MaterialShaderEquation* CreateMaterialShaderEquation();
	MaterialShaderGraph* CreateMaterialShaderGraph();
	Font* CreateFont();
	
	// Scene
	Scene* CreateScene(std::string name);
	MeshEntity* CreateMeshEntity();
	OverlayEntity* CreateOverlayEntity();
	TextEntity* CreateTextEntity();
	PerspectiveCamera* CreatePerspectiveCamera(std::string name);
	OrthographicCamera* CreateOrthographicCamera(std::string name);
	Camera2D* CreateCamera2D(std::string name);


	// Pipeline and environment variables

	/// <summary> Creates or sets an environment variable to the given value. </summary>
	/// <returns> True if a new variable was created, false if old was overridden. </returns>
	/// <remarks> Environment variables can be accessed in the graphics pipeline graph by the special
	///		<see cref="nodes::GetEnvVariable"/> node. You can use it to slightly 
	///		alter pipeline behavriour from outside. </remarks>
	bool SetEnvVariable(std::string name, Any obj);

	/// <summary> Returns true if env var with given name exists. </summary>
	bool EnvVariableExists(const std::string& name);

	/// <summary> Return the env var with given name or throws <see cref="InvalidArgumentException"/>. </summary>
	const Any& GetEnvVariable(const std::string& name);

	/// <summary> Load the pipeline from the JSON node graph description. </summary>
	/// <remarks> Tears down all the resources associated with the old pipeline, including
	///		textures, render targets, etc., and builds up the new pipeline.
	///		Also incurs a pipeline queue flush. Use it only when settings change,
	///		use env vars to control pipeline behaviour on the fly.
	void LoadPipeline(const std::string& nodes);

	/// <summary> The engine will look for shader files in these directories. </summary>
	/// <remarks> May be absolute, relative, or whatever paths you OS can handle. </remarks>
	void SetShaderDirectories(const std::vector<std::experimental::filesystem::path>& directories);
private:
	void FlushPipelineQueue();
	void RegisterPipelineClasses();
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
	GraphicsNodeFactory m_nodeFactory;
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
	std::set<Camera2D*> m_cameras2d;
};



} // namespace gxeng
} // namespace inl