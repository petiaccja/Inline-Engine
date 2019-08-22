#pragma once

#include "BackBufferManager.hpp"
#include "CommandAllocatorPool.hpp"
#include "CommandListPool.hpp"
#include "HostDescHeap.hpp"
#include "MemoryManager.hpp"
#include "Pipeline.hpp"
#include "PipelineEventDispatcher.hpp"
#include "ResourceResidencyQueue.hpp"
#include "Scheduler.hpp"
#include "ScratchSpacePool.hpp"
#include "ShaderManager.hpp"

#include <BaseLibrary/Any.hpp>
#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>
#include <GraphicsApi_LL/IGxapiManager.hpp>
#include <GraphicsApi_LL/ISwapChain.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>

#include <filesystem>

// For the create functions, type covariance.
#include "Camera2D.hpp"
#include "DirectionalLight.hpp"
#include "Font.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "MeshEntity.hpp"
#include "OrthographicCamera.hpp"
#include "OverlayEntity.hpp"
#include "PerspectiveCamera.hpp"
#include "Scene.hpp"
#include "TextEntity.hpp"


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


	struct GraphicsEngineDesc {
		gxapi::IGxapiManager* gxapiManager = nullptr;
		gxapi::IGraphicsApi* graphicsApi = nullptr;
		gxapi::NativeWindowHandle targetWindow = {};
		bool fullScreen = false;
		int width = 640;
		int height = 480;
	};


	class GraphicsEngine : public IGraphicsEngine {
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
		void Update(float elapsed) override;

		/// <summary> Rescales the backbuffer. </summary>
		/// <remarks> Causes a pipeline flush, high overhead. </remarks>
		void SetScreenSize(unsigned width, unsigned height) override;

		/// <summary> Returns the current backbuffer size in the out parameters. </remarks>
		void GetScreenSize(unsigned& width, unsigned& height) override;

		/// <summary> Sets the D3D swap chain to full-screen mode. </summary>
		/// <param name="enable"> True to full screen, false to windowed. </param>
		void SetFullScreen(bool enable) override;

		/// <summary> True if the swap chain is currently in full-screen mode. </summary>
		bool GetFullScreen() const override;


		// Graph editor interfaces
		std::unique_ptr<IEditorGraph> QueryPipelineEditor() const override;
		std::unique_ptr<IEditorGraph> QueryMaterialEditor() const override;


		// Resources
		std::unique_ptr<IMesh> CreateMesh() const override;
		std::unique_ptr<IImage> CreateImage() const override;
		std::unique_ptr<IMaterial> CreateMaterial() const override;
		std::unique_ptr<IMaterialShaderEquation> CreateMaterialShaderEquation() const override;
		std::unique_ptr<IMaterialShaderGraph> CreateMaterialShaderGraph() const override;
		std::unique_ptr<IFont> CreateFont() const override;

		// Scene
		std::unique_ptr<IScene> CreateScene(std::string name) override;
		std::unique_ptr<IMeshEntity> CreateMeshEntity() const override;
		std::unique_ptr<IOverlayEntity> CreateOverlayEntity() const override;
		std::unique_ptr<ITextEntity> CreateTextEntity() const override;
		std::unique_ptr<IPerspectiveCamera> CreatePerspectiveCamera(std::string name) override;
		std::unique_ptr<IOrthographicCamera> CreateOrthographicCamera(std::string name) override;
		std::unique_ptr<ICamera2D> CreateCamera2D(std::string name) override;
		std::unique_ptr<IDirectionalLight> CreateDirectionalLight() const override;

		// Pipeline and environment variables

		/// <summary> Creates or sets an environment variable to the given value. </summary>
		/// <returns> True if a new variable was created, false if old was overridden. </returns>
		/// <remarks> Environment variables can be accessed in the graphics pipeline graph by the special
		///		<see cref="nodes::GetEnvVariable"/> node. You can use it to slightly
		///		alter pipeline behavriour from outside. </remarks>
		bool SetEnvVariable(std::string name, Any obj) override;

		/// <summary> Returns true if env var with given name exists. </summary>
		bool EnvVariableExists(const std::string& name) override;

		/// <summary> Return the env var with given name or throws <see cref="InvalidArgumentException"/>. </summary>
		const Any& GetEnvVariable(const std::string& name) override;

		/// <summary> Load the pipeline from the JSON node graph description. </summary>
		/// <remarks> Tears down all the resources associated with the old pipeline, including
		///		textures, render targets, etc., and builds up the new pipeline.
		///		Also incurs a pipeline queue flush. Use it only when settings change,
		///		use env vars to control pipeline behaviour on the fly.
		void LoadPipeline(const std::string& nodes) override;

		/// <summary> The engine will look for shader files in these directories. </summary>
		/// <remarks> May be absolute, relative, or whatever paths you OS can handle. </remarks>
		void SetShaderDirectories(const std::vector<std::filesystem::path>& directories) override;

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