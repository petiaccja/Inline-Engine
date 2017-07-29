#include "GraphicsEngine.hpp"
#include "GraphicsNode.hpp"

#include "../BaseLibrary/Graph/Node.hpp"

#include <iostream> // only for debugging
#include <regex> // as well...
#include <lemon/bfs.h> // as well...

#include "Nodes/Node_GetBackBuffer.hpp"
#include "Nodes/Node_TextureProperties.hpp"
#include "Nodes/Node_CreateTexture.hpp"
#include "Nodes/Node_GetSceneByName.hpp"
#include "Nodes/Node_GetCameraByName.hpp"
#include "Nodes/Node_GetTime.hpp"
#include "Nodes/Node_GetEnvVariable.hpp"
#include "Nodes/Node_VectorComponents.hpp"


//forward
#include "Nodes/Node_ForwardRender.hpp"
#include "Nodes/Node_DepthPrepass.hpp"
#include "Nodes/Node_DepthReduction.hpp"
#include "Nodes/Node_DepthReductionFinal.hpp"
#include "Nodes/Node_CSM.hpp"
#include "Nodes/Node_DrawSky.hpp"
#include "Nodes/Node_DebugDraw.hpp"
#include "Nodes/Node_LightCulling.hpp"
#include "Nodes/Node_BrightLumPass.hpp"
#include "Nodes/Node_LuminanceReduction.hpp"
#include "Nodes/Node_LuminanceReductionFinal.hpp"
#include "Nodes/Node_HDRCombine.hpp"
#include "Nodes/Node_BloomDownsample.hpp"
#include "Nodes/Node_BloomBlur.hpp"
#include "Nodes/Node_BloomAdd.hpp"
#include "Nodes/Node_TileMax.hpp"
#include "Nodes/Node_NeighborMax.hpp"
#include "Nodes/Node_MotionBlur.hpp"
#include "Nodes/Node_LensFlare.hpp"
#include "Nodes/Node_SMAA.hpp"
#include "Nodes/Node_DOFPrepare.hpp"
#include "Nodes/Node_DOFTileMax.hpp"
#include "Nodes/Node_DOFNeighborMax.hpp"

//Gui
#include "Nodes/Node_OverlayRender.hpp"
#include "Nodes/Node_Blend.hpp"
#include "Nodes/Node_ScreenSpaceTransform.hpp"
#include "Nodes/Node_BlendWithTransform.hpp"

#include "Scene.hpp"
#include "PerspectiveCamera.hpp"
#include "OrthographicCamera.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Image.hpp"
#include "MeshEntity.hpp"
#include "OverlayEntity.hpp"


namespace inl {
namespace gxeng {

using namespace gxapi;



GraphicsEngine::GraphicsEngine(GraphicsEngineDesc desc)
	: m_gxapiManager(desc.gxapiManager),
	m_graphicsApi(desc.graphicsApi),
	m_commandAllocatorPool(desc.graphicsApi),
	m_commandListPool(desc.graphicsApi),
	m_scratchSpacePool(desc.graphicsApi, gxapi::eDescriptorHeapType::CBV_SRV_UAV),
	m_textureSpace(desc.graphicsApi),
	m_masterCommandQueue(desc.graphicsApi->CreateCommandQueue(CommandQueueDesc{ eCommandListType::GRAPHICS }), desc.graphicsApi->CreateFence(0)),
	m_residencyQueue(std::unique_ptr<gxapi::IFence>(desc.graphicsApi->CreateFence(0))),
	m_memoryManager(desc.graphicsApi),
	m_dsvHeap(desc.graphicsApi),
	m_rtvHeap(desc.graphicsApi),
	m_persResViewHeap(desc.graphicsApi),
	m_logger(desc.logger),
	m_shaderManager(desc.gxapiManager)
{
	// Create swapchain
	SwapChainDesc swapChainDesc;
	swapChainDesc.format = eFormat::R8G8B8A8_UNORM;
	swapChainDesc.width = desc.width;
	swapChainDesc.height = desc.height;
	swapChainDesc.numBuffers = 2;
	swapChainDesc.targetWindow = desc.targetWindow;
	swapChainDesc.isFullScreen = desc.fullScreen;
	swapChainDesc.multisampleCount = 1;
	swapChainDesc.multiSampleQuality = 0;
	m_swapChain.reset(m_gxapiManager->CreateSwapChain(swapChainDesc, m_masterCommandQueue.GetUnderlyingQueue()));

	m_frameEndFenceValues.resize(m_swapChain->GetDesc().numBuffers, { nullptr, 0 });

	// Init backbuffer heap
	m_backBufferHeap = std::make_unique<BackBufferManager>(m_graphicsApi, m_swapChain.get());

	// Init shader manager before creating the pipeline
	m_shaderManager.AddSourceDirectory("../../Engine/GraphicsEngine_LL/Nodes/Shaders");
	m_shaderManager.AddSourceDirectory("../../Engine/GraphicsEngine_LL/Materials");
	m_shaderManager.AddSourceDirectory("./Shaders");
	m_shaderManager.AddSourceDirectory("./Materials");
	gxapi::eShaderCompileFlags shaderFlags;
	shaderFlags += gxapi::eShaderCompileFlags::ROW_MAJOR_MATRICES;
#ifdef NDEBUG
	shaderFlags += gxapi::eShaderCompileFlags::OPTIMIZATION_HIGH;
#else
	shaderFlags += gxapi::eShaderCompileFlags::NO_OPTIMIZATION;
	shaderFlags += gxapi::eShaderCompileFlags::DEBUG;
#endif // NDEBUG
	m_shaderManager.SetShaderCompileFlags(shaderFlags);


	// Do more stuff...
	CreatePipeline();
	m_scheduler.SetPipeline(std::move(m_pipeline));

	// Init logger
	m_logStreamGeneral = m_logger->CreateLogStream("General");
	m_logStreamPipeline = m_logger->CreateLogStream("Pipeline");

	// Init misc stuff
	m_absoluteTime = decltype(m_absoluteTime)(0);
	m_commandAllocatorPool.SetLogStream(&m_logStreamPipeline);

	m_pipelineEventDispatcher += &m_memoryManager.GetUploadManager();
	// DELETE THIS
	m_pipelineEventPrinter.SetLog(&m_logStreamPipeline);
	m_pipelineEventDispatcher += &m_pipelineEventPrinter;



	// Begin awaiting frame #0's Update()
	m_pipelineEventDispatcher.DispachFrameBeginAwait(0);
}


GraphicsEngine::~GraphicsEngine() {
	std::cout << "Graphics engine shutting down..." << std::endl;
	SyncPoint lastSync = m_masterCommandQueue.Signal();
	lastSync.Wait();
	std::cout << "Graphics engine deleting..." << std::endl;
}


void GraphicsEngine::Update(float elapsed) {
	std::chrono::nanoseconds frameTime(long long(elapsed * 1e9));
	m_absoluteTime += frameTime;

	// Wait for previous frame on this BB to complete
	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	if (m_frameEndFenceValues[backBufferIndex]) {
		m_frameEndFenceValues[backBufferIndex].Wait();
	}

	// Set up context
	FrameContext context;
	context.frameTime = frameTime;
	context.absoluteTime = m_absoluteTime;
	context.log = &m_logStreamPipeline;
	context.frame = m_frame;

	context.gxApi = m_graphicsApi;
	context.commandAllocatorPool = &m_commandAllocatorPool;
	context.commandListPool = &m_commandListPool;
	context.scratchSpacePool = &m_scratchSpacePool;
	context.memoryManager = &m_memoryManager;
	context.textureSpace = &m_textureSpace;
	context.rtvHeap = &m_rtvHeap;
	context.dsvHeap = &m_dsvHeap;
	context.shaderManager = &m_shaderManager;

	context.commandQueue = &m_masterCommandQueue;
	context.backBuffer = &m_backBufferHeap->GetBackBuffer(backBufferIndex);
	context.scenes = &m_scenes;
	context.cameras = &m_cameras;

	const std::vector<UploadManager::UploadDescription>& uploadRequests = m_memoryManager.GetUploadManager().GetQueuedUploads();
	context.uploadRequests = &uploadRequests;

	context.residencyQueue = &m_residencyQueue;

	// Update special nodes for current frame
	UpdateSpecialNodes();

	// Execute the pipeline
	m_pipelineEventDispatcher.DispatchFrameBegin(m_frame).wait();
	m_scheduler.Execute(context);
	m_pipelineEventDispatcher.DispatchFrameEnd(m_frame).wait();

	// Mark frame completion
	SyncPoint frameEnd = m_masterCommandQueue.Signal();
	m_frameEndFenceValues[backBufferIndex] = frameEnd;
	m_pipelineEventDispatcher.DispatchDeviceFrameEnd(frameEnd, m_frame);

	// Flush log
	m_logger->Flush();

	// Present frame
	m_swapChain->Present();
	++m_frame;

	// Await next frame
	m_pipelineEventDispatcher.DispachFrameBeginAwait(m_frame).wait(); // m_frame incremented on previous line
}


void GraphicsEngine::SetScreenSize(unsigned width, unsigned height) {
	if (width == 0 || height == 0) {
		return;
	}

	SyncPoint sp = m_masterCommandQueue.Signal();
	sp.Wait();

	m_backBufferHeap.reset();
	m_scheduler.ReleaseResources();
	m_swapChain->Resize(width, height);
	m_backBufferHeap = std::make_unique<BackBufferManager>(m_graphicsApi, m_swapChain.get());
}
void GraphicsEngine::GetScreenSize(unsigned& width, unsigned& height) {
	auto desc = m_swapChain->GetDesc();
	width = desc.width;
	height = desc.height;
}


void GraphicsEngine::SetFullScreen(bool enable) {
	m_swapChain->SetFullScreen(enable);
}
bool GraphicsEngine::GetFullScreen() const {
	return m_swapChain->IsFullScreen();
}


// Resources
Mesh* GraphicsEngine::CreateMesh() {
	return new Mesh(&m_memoryManager);
}

Image* GraphicsEngine::CreateImage() {
	return new Image(&m_memoryManager, &m_textureSpace);
}

Material* GraphicsEngine::CreateMaterial() {
	return new Material;
}

MaterialShaderEquation* GraphicsEngine::CreateMaterialShaderEquation() {
	return new MaterialShaderEquation(&m_shaderManager);
}

MaterialShaderGraph* GraphicsEngine::CreateMaterialShaderGraph() {
	return new MaterialShaderGraph(&m_shaderManager);
}



// Scene
Scene* GraphicsEngine::CreateScene(std::string name) {
	// Declare a derived class for the sole purpose of making the destructor unregister the object from scene list.
	class ObservedScene : public Scene {
	public:
		ObservedScene(std::function<void(Scene*)> deleteHandler, std::string name) :
			Scene(std::move(name)), m_deleteHandler(std::move(deleteHandler))
		{}
		~ObservedScene() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<Scene*>(this)); }
		}
	protected:
		std::function<void(Scene*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterScene = [this](Scene* arg) {
		m_scenes.erase(arg);
	};

	// Allocate a new scene, and register it.
	Scene* scene = new ObservedScene(unregisterScene, std::move(name));
	m_scenes.insert(scene);

	return scene;
}


PerspectiveCamera* GraphicsEngine::CreatePerspectiveCamera(std::string name) {
	class ObservedPerspectiveCamera : public PerspectiveCamera {
	public:
		ObservedPerspectiveCamera(std::function<void(PerspectiveCamera*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler))
		{
			SetName(name);
		}
		~ObservedPerspectiveCamera() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<PerspectiveCamera*>(this)); }
		}
	protected:
		std::function<void(PerspectiveCamera*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](PerspectiveCamera* arg) {
		m_cameras.erase(arg);
	};

	// Allocate a new scene, and register it.
	PerspectiveCamera* camera = new ObservedPerspectiveCamera(unregisterCamera, std::move(name));
	m_cameras.insert(camera);

	return camera;
}

OrthographicCamera* GraphicsEngine::CreateOrthographicCamera(std::string name) {
	class ObservedOrthographicCamera : public OrthographicCamera {
	public:
		ObservedOrthographicCamera(std::function<void(OrthographicCamera*)> deleteHandler, std::string name) :
			m_deleteHandler(std::move(deleteHandler)) {
			SetName(name);
		}
		~ObservedOrthographicCamera() {
			if (m_deleteHandler) { m_deleteHandler(static_cast<OrthographicCamera*>(this)); }
		}
	protected:
		std::function<void(OrthographicCamera*)> m_deleteHandler;
	};

	// Functor to perform the unregistration.
	auto unregisterCamera = [this](OrthographicCamera* arg) {
		m_cameras.erase(arg);
	};

	// Allocate a new scene, and register it.
	OrthographicCamera* camera = new ObservedOrthographicCamera(unregisterCamera, std::move(name));
	m_cameras.insert(camera);

	return camera;
}

MeshEntity* GraphicsEngine::CreateMeshEntity() {
	return new MeshEntity;
}

OverlayEntity* GraphicsEngine::CreateOverlayEntity() {
	return new OverlayEntity;
}


bool GraphicsEngine::SetEnvVariable(std::string name, Any obj) {
	auto res = m_envVariables.insert_or_assign(std::move(name), std::move(obj));
	return res.second;
}

bool GraphicsEngine::EnvVariableExists(const std::string& name) {
	return m_envVariables.count(name) > 0;
}

const Any& GraphicsEngine::GetEnvVariable(const std::string& name) {
	auto it = m_envVariables.find(name);
	if (it != m_envVariables.end()) {
		return it->second;
	}
	else {
		throw InvalidArgumentException("Environment variable does not exist.");
	}
}



void GraphicsEngine::CreatePipeline() {
	auto swapChainDesc = m_swapChain->GetDesc();

	// -----------------------------
	// 3D pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getWorldScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::GetBackBuffer> getBackBuffer(new nodes::GetBackBuffer());
	
	getWorldScene->GetInput<0>().Set("World");
	getCamera->GetInput<0>().Set("WorldCam");

	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderPos(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderRot(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::GetEnvVariable> getWorldRenderSize(new nodes::GetEnvVariable());
	std::shared_ptr<nodes::VectorComponents<2>> worldRenderSizeSplit(new nodes::VectorComponents<2>());

	getWorldRenderPos->SetEnvVariableList(&m_envVariables);
	getWorldRenderRot->SetEnvVariableList(&m_envVariables);
	getWorldRenderSize->SetEnvVariableList(&m_envVariables);

	getWorldRenderPos->GetInput<0>().Set("world_render_pos");
	getWorldRenderRot->GetInput<0>().Set("world_render_rot");
	getWorldRenderSize->GetInput<0>().Set("world_render_size");

	// TODO set the render buffers size to match "world_render_size"
	if (!worldRenderSizeSplit->GetInput<0>().Link(getWorldRenderSize->GetOutput(0))) 		{
		assert(false);
	}

	std::shared_ptr<nodes::TextureProperties> backBufferProperties(new nodes::TextureProperties());
	std::shared_ptr<nodes::CreateTexture> createDepthBuffer(new nodes::CreateTexture());
	std::shared_ptr<nodes::CreateTexture> createHdrRenderTarget(new nodes::CreateTexture());
	std::shared_ptr<nodes::CreateTexture> createCsmTextures(new nodes::CreateTexture());
	std::shared_ptr<nodes::ForwardRender> forwardRender(new nodes::ForwardRender());
	std::shared_ptr<nodes::DepthPrepass> depthPrePass(new nodes::DepthPrepass());
	std::shared_ptr<nodes::DepthReduction> depthReduction(new nodes::DepthReduction());
	std::shared_ptr<nodes::DepthReductionFinal> depthReductionFinal(new nodes::DepthReductionFinal());
	std::shared_ptr<nodes::CSM> csm(new nodes::CSM());
	std::shared_ptr<nodes::DrawSky> drawSky(new nodes::DrawSky());
	std::shared_ptr<nodes::DebugDraw> debugDraw(new nodes::DebugDraw());
	std::shared_ptr<nodes::LightCulling> lightCulling(new nodes::LightCulling());
	std::shared_ptr<nodes::BrightLumPass> brightLumPass(new nodes::BrightLumPass());
	std::shared_ptr<nodes::LuminanceReduction> luminanceReduction(new nodes::LuminanceReduction());
	std::shared_ptr<nodes::LuminanceReductionFinal> luminanceReductionFinal(new nodes::LuminanceReductionFinal());
	std::shared_ptr<nodes::HDRCombine> hdrCombine(new nodes::HDRCombine());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample2(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample4(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample8(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample16(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomDownsample> bloomDownsample32(new nodes::BloomDownsample());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical32(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical16(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical8(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical4(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurVertical2(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal32(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal16(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal8(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal4(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> bloomBlurHorizontal2(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomAdd> bloomAdd3216(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd168(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd84(new nodes::BloomAdd());
	std::shared_ptr<nodes::BloomAdd> bloomAdd42(new nodes::BloomAdd());
	std::shared_ptr<nodes::TileMax> tileMax(new nodes::TileMax());
	std::shared_ptr<nodes::NeighborMax> neighborMax(new nodes::NeighborMax());
	std::shared_ptr<nodes::MotionBlur> motionBlur(new nodes::MotionBlur());
	std::shared_ptr<nodes::LensFlare> lensFlare(new nodes::LensFlare());
	std::shared_ptr<nodes::BloomBlur> lensFlareBlurHorizontal(new nodes::BloomBlur());
	std::shared_ptr<nodes::BloomBlur> lensFlareBlurVertical(new nodes::BloomBlur());
	auto smaaAreaEnv = std::make_shared<nodes::GetEnvVariable>();
	auto smaaSearchEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareColorEnv = std::make_shared<nodes::GetEnvVariable>();
	auto colorGradingEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareDirtEnv = std::make_shared<nodes::GetEnvVariable>();
	auto lensFlareStarEnv = std::make_shared<nodes::GetEnvVariable>();
	std::shared_ptr<nodes::SMAA> smaa(new nodes::SMAA());
	std::shared_ptr<nodes::DOFPrepare> dofPrepare(new nodes::DOFPrepare());
	std::shared_ptr<nodes::DOFTileMax> dofTileMax(new nodes::DOFTileMax());
	std::shared_ptr<nodes::DOFNeighborMax> dofNeighborMax(new nodes::DOFNeighborMax());
	TextureUsage usage;


	backBufferProperties->GetInput<0>().Link(getBackBuffer->GetOutput(0));

	createDepthBuffer->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createDepthBuffer->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createDepthBuffer->GetInput<2>().Set(gxapi::eFormat::R32G8X24_TYPELESS);
	createDepthBuffer->GetInput<3>().Set(1);
	usage = TextureUsage();
	usage.depthStencil = true;
	createDepthBuffer->GetInput<4>().Set(usage);

	depthPrePass->GetInput(0)->Link(createDepthBuffer->GetOutput(0));
	depthPrePass->GetInput(1)->Link(getWorldScene->GetOutput(0));
	depthPrePass->GetInput(2)->Link(getCamera->GetOutput(0));

	depthReduction->GetInput<0>().Link(depthPrePass->GetOutput(0));

	depthReductionFinal->GetInput<0>().Link(depthReduction->GetOutput(0));
	depthReductionFinal->GetInput<1>().Link(getCamera->GetOutput(0));
	depthReductionFinal->GetInput<2>().Link(getWorldScene->GetOutput(2));

	constexpr unsigned cascadeSize = 1024;
	constexpr unsigned numCascades = 4;

	createCsmTextures->GetInput<0>().Set(cascadeSize);
	createCsmTextures->GetInput<1>().Set(cascadeSize);
	createCsmTextures->GetInput<2>().Set(gxapi::eFormat::R32_TYPELESS);
	createCsmTextures->GetInput<3>().Set(numCascades);
	usage = TextureUsage();
	usage.depthStencil = true;
	createCsmTextures->GetInput<4>().Set(usage);

	csm->GetInput<0>().Link(createCsmTextures->GetOutput(0));
	csm->GetInput<1>().Link(getWorldScene->GetOutput(0));
	csm->GetInput<2>().Link(depthReductionFinal->GetOutput(0));

	//TODO (2.5D light culling + verify)
	lightCulling->GetInput<0>().Link(depthPrePass->GetOutput(0));
	lightCulling->GetInput<1>().Link(getCamera->GetOutput(0));

	createHdrRenderTarget->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createHdrRenderTarget->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createHdrRenderTarget->GetInput<2>().Set(gxapi::eFormat::R16G16B16A16_FLOAT);
	createHdrRenderTarget->GetInput<3>().Set(1);
	usage = TextureUsage();
	usage.renderTarget = true;
	createHdrRenderTarget->GetInput<4>().Set(usage);

	forwardRender->GetInput(0)->Link(createHdrRenderTarget->GetOutput(0));
	forwardRender->GetInput(1)->Link(depthPrePass->GetOutput(0));
	forwardRender->GetInput(2)->Link(getWorldScene->GetOutput(0));
	forwardRender->GetInput(3)->Link(getCamera->GetOutput(0));
	forwardRender->GetInput(4)->Link(getWorldScene->GetOutput(2));
	forwardRender->GetInput(5)->Link(csm->GetOutput(0));
	forwardRender->GetInput(6)->Link(depthReductionFinal->GetOutput(1));
	forwardRender->GetInput(7)->Link(depthReductionFinal->GetOutput(2));
	forwardRender->GetInput(8)->Link(depthReductionFinal->GetOutput(0));
	forwardRender->GetInput(9)->Link(lightCulling->GetOutput(0));

	drawSky->GetInput<0>().Link(forwardRender->GetOutput(0));
	drawSky->GetInput<1>().Link(depthPrePass->GetOutput(0));
	drawSky->GetInput<2>().Link(getCamera->GetOutput(0));
	drawSky->GetInput<3>().Link(getWorldScene->GetOutput(2));

	tileMax->GetInput<0>().Link(forwardRender->GetOutput(1));
	
	neighborMax->GetInput<0>().Link(tileMax->GetOutput(0));

	motionBlur->GetInput<0>().Link(drawSky->GetOutput(0));
	motionBlur->GetInput<1>().Link(forwardRender->GetOutput(1));
	motionBlur->GetInput<2>().Link(neighborMax->GetOutput(0));
	motionBlur->GetInput<3>().Link(depthPrePass->GetOutput(0));

	dofPrepare->GetInput<0>().Link(motionBlur->GetOutput(0));
	dofPrepare->GetInput<1>().Link(depthPrePass->GetOutput(0));
	dofPrepare->GetInput<2>().Link(getCamera->GetOutput(0));

	dofTileMax->GetInput<0>().Link(dofPrepare->GetOutput(0));
	dofTileMax->GetInput<1>().Link(depthPrePass->GetOutput(0));

	dofNeighborMax->GetInput<0>().Link(dofTileMax->GetOutput(0));

	brightLumPass->GetInput<0>().Link(motionBlur->GetOutput(0));
	
	bloomDownsample2->GetInput<0>().Link(brightLumPass->GetOutput(0));
	bloomDownsample4->GetInput<0>().Link(bloomDownsample2->GetOutput(0));
	bloomDownsample8->GetInput<0>().Link(bloomDownsample4->GetOutput(0));
	bloomDownsample16->GetInput<0>().Link(bloomDownsample8->GetOutput(0));
	bloomDownsample32->GetInput<0>().Link(bloomDownsample16->GetOutput(0));

	bloomBlurVertical32->GetInput<0>().Link(bloomDownsample32->GetOutput(0));
	bloomBlurVertical32->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal32->GetInput<0>().Link(bloomBlurVertical32->GetOutput(0));
	bloomBlurHorizontal32->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd3216->GetInput<0>().Link(bloomBlurHorizontal32->GetOutput(0));
	bloomAdd3216->GetInput<1>().Link(bloomDownsample16->GetOutput(0));

	bloomBlurVertical16->GetInput<0>().Link(bloomAdd3216->GetOutput(0));
	bloomBlurVertical16->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal16->GetInput<0>().Link(bloomBlurVertical16->GetOutput(0));
	bloomBlurHorizontal16->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd168->GetInput<0>().Link(bloomBlurHorizontal16->GetOutput(0));
	bloomAdd168->GetInput<1>().Link(bloomDownsample8->GetOutput(0));

	bloomBlurVertical8->GetInput<0>().Link(bloomAdd168->GetOutput(0));
	bloomBlurVertical8->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal8->GetInput<0>().Link(bloomBlurVertical8->GetOutput(0));
	bloomBlurHorizontal8->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd84->GetInput<0>().Link(bloomBlurHorizontal8->GetOutput(0));
	bloomAdd84->GetInput<1>().Link(bloomDownsample4->GetOutput(0));

	bloomBlurVertical4->GetInput<0>().Link(bloomAdd84->GetOutput(0));
	bloomBlurVertical4->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal4->GetInput<0>().Link(bloomBlurVertical4->GetOutput(0));
	bloomBlurHorizontal4->GetInput<1>().Set(Vec2(1, 0));
	bloomAdd42->GetInput<0>().Link(bloomBlurHorizontal4->GetOutput(0));
	bloomAdd42->GetInput<1>().Link(bloomDownsample2->GetOutput(0));

	bloomBlurVertical2->GetInput<0>().Link(bloomAdd42->GetOutput(0));
	bloomBlurVertical2->GetInput<1>().Set(Vec2(0, 1));
	bloomBlurHorizontal2->GetInput<0>().Link(bloomBlurVertical2->GetOutput(0));
	bloomBlurHorizontal2->GetInput<1>().Set(Vec2(1, 0));

	lensFlareColorEnv->GetInput<0>().Set("LensFlare_ColorTex");
	lensFlare->GetInput<0>().Link(bloomDownsample4->GetOutput(0));
	lensFlare->GetInput<1>().Link(lensFlareColorEnv->GetOutput(0));

	lensFlareBlurVertical->GetInput<0>().Link(lensFlare->GetOutput(0));
	lensFlareBlurVertical->GetInput<1>().Set(Vec2(0, 1));
	lensFlareBlurHorizontal->GetInput<0>().Link(lensFlareBlurVertical->GetOutput(0));
	lensFlareBlurHorizontal->GetInput<1>().Set(Vec2(1, 0));

	luminanceReduction->GetInput<0>().Link(brightLumPass->GetOutput(1));

	luminanceReductionFinal->GetInput<0>().Link(luminanceReduction->GetOutput(0));

	colorGradingEnv->GetInput<0>().Set("HDRCombine_colorGradingTex");
	lensFlareDirtEnv->GetInput<0>().Set("HDRCombine_lensFlareDirtTex");
	lensFlareStarEnv->GetInput<0>().Set("HDRCombine_lensFlareStarTex");
	hdrCombine->GetInput<0>().Link(motionBlur->GetOutput(0));
	hdrCombine->GetInput<1>().Link(luminanceReductionFinal->GetOutput(0));
	hdrCombine->GetInput<2>().Link(bloomBlurHorizontal2->GetOutput(0));
	hdrCombine->GetInput<3>().Link(lensFlareBlurHorizontal->GetOutput(0));
	hdrCombine->GetInput<4>().Link(colorGradingEnv->GetOutput(0));
	hdrCombine->GetInput<5>().Link(lensFlareDirtEnv->GetOutput(0));
	hdrCombine->GetInput<6>().Link(lensFlareStarEnv->GetOutput(0));
	hdrCombine->GetInput<7>().Link(getCamera->GetOutput(0));

	// last step in world render is debug draw
	//debugDraw->GetInput<0>().Link(drawSky->GetOutput(0));
	//debugDraw->GetInput<0>().Link(forwardRender->GetOutput(1));
	debugDraw->GetInput<0>().Link(hdrCombine->GetOutput(0));
	debugDraw->GetInput<1>().Link(getCamera->GetOutput(0));

	smaaAreaEnv->GetInput<0>().Set("SMAA_areaTex");
	smaaSearchEnv->GetInput<0>().Set("SMAA_searchTex");
	smaa->GetInput<0>().Link(debugDraw->GetOutput(0));
	smaa->GetInput(1)->Link(smaaAreaEnv->GetOutput(0));
	smaa->GetInput<2>().Link(smaaSearchEnv->GetOutput(0));


	// -----------------------------
	// Gui pipeline path
	// -----------------------------
	std::shared_ptr<nodes::GetSceneByName> getGuiScene(new nodes::GetSceneByName());
	std::shared_ptr<nodes::GetCameraByName> getGuiCamera(new nodes::GetCameraByName());
	std::shared_ptr<nodes::OverlayRender> guiRender(new nodes::OverlayRender());
	std::shared_ptr<nodes::BlendWithTransform> alphaBlend(new nodes::BlendWithTransform());
	std::shared_ptr<nodes::ScreenSpaceTransform> createWorldRenderTransform(new nodes::ScreenSpaceTransform());

	getGuiScene->GetInput<0>().Set("Gui");
	getGuiCamera->GetInput<0>().Set("GuiCamera");

	createWorldRenderTransform->GetInput<0>().Link(backBufferProperties->GetOutput(0));
	createWorldRenderTransform->GetInput<1>().Link(backBufferProperties->GetOutput(1));
	createWorldRenderTransform->GetInput<2>().Link(getWorldRenderPos->GetOutput(0));
	createWorldRenderTransform->GetInput<3>().Link(getWorldRenderRot->GetOutput(0));
	createWorldRenderTransform->GetInput<4>().Link(getWorldRenderSize->GetOutput(0));

	//createWorldRenderTransform->GetInput<0>().Set(800);
	//createWorldRenderTransform->GetInput<1>().Set(600);
	//createWorldRenderTransform->GetInput<2>().Set(Vec2(0.f, 0.f));
	//createWorldRenderTransform->GetInput<3>().Set(0);
	//createWorldRenderTransform->GetInput<4>().Set(Vec2(800.f, 600.f));

	guiRender->GetInput<0>().Link(getBackBuffer->GetOutput(0));
	guiRender->GetInput<1>().Link(getGuiScene->GetOutput(1));
	guiRender->GetInput<2>().Link(getGuiCamera->GetOutput(0));

	// NOTE: the intended behaviour of this blending is to draw the render target on top of the shader output (render target contains overlay, shader output contains scene)
	gxapi::RenderTargetBlendState blending;
	blending.enableBlending = true;
	blending.alphaOperation = gxapi::eBlendOperation::ADD;
	blending.shaderAlphaFactor = gxapi::eBlendOperand::INV_TARGET_ALPHA;
	blending.targetAlphaFactor = gxapi::eBlendOperand::TARGET_ALPHA;
	blending.colorOperation = gxapi::eBlendOperation::ADD;
	blending.shaderColorFactor = gxapi::eBlendOperand::INV_TARGET_ALPHA;
	blending.targetColorFactor = gxapi::eBlendOperand::TARGET_ALPHA;
	blending.enableLogicOp = false;
	blending.mask = gxapi::eColorMask::ALL;

	alphaBlend->GetInput<0>().Link(guiRender->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(debugDraw->GetOutput(0));
	alphaBlend->GetInput<1>().Link(smaa->GetOutput(0));
	//alphaBlend->GetInput<1>().Link(dofPrepare->GetOutput(0));
	alphaBlend->GetInput<2>().Set(blending);
	//alphaBlend->GetInput<3>().Set(Mat44::FromScaleVector(Vec3(.5f, 1.f, 1.f)));
	alphaBlend->GetInput<3>().Link(createWorldRenderTransform->GetOutput(0));

	m_graphicsNodes = {
		getWorldScene,
		getCamera,
		getBackBuffer,

		getWorldRenderPos,
		getWorldRenderRot,
		getWorldRenderSize,
		worldRenderSizeSplit,

		backBufferProperties,
		createDepthBuffer,
		createHdrRenderTarget,
		createCsmTextures,
		forwardRender,
		depthPrePass,
		depthReduction,
		depthReductionFinal,
		csm,
		drawSky,
		lightCulling,
		brightLumPass,
		luminanceReduction,
		luminanceReductionFinal,
		bloomDownsample2,
		bloomDownsample4,
		bloomDownsample8,
		bloomDownsample16,
		bloomDownsample32,
		bloomBlurVertical32,
		bloomBlurHorizontal32,
		bloomAdd3216,
		bloomBlurVertical16,
		bloomBlurHorizontal16,
		bloomAdd168,
		bloomBlurVertical8,
		bloomBlurHorizontal8,
		bloomAdd84,
		bloomBlurVertical4,
		bloomBlurHorizontal4,
		bloomAdd42,
		bloomBlurVertical2,
		bloomBlurHorizontal2,
		hdrCombine,
		tileMax,
		neighborMax,
		motionBlur,
		lensFlare,
		lensFlareBlurHorizontal,
		lensFlareBlurVertical,
		smaa,
		dofPrepare,
		dofTileMax,
		dofNeighborMax,
		smaaAreaEnv,
		smaaSearchEnv,
		lensFlareColorEnv,
		colorGradingEnv,
		lensFlareDirtEnv,
		lensFlareStarEnv,


		getGuiScene,
		getGuiCamera,
		guiRender,
		//alphaBlend,
		createWorldRenderTransform,
		debugDraw,
		alphaBlend
	};


	std::vector<std::shared_ptr<NodeBase>> nodeList;
	nodeList.reserve(m_graphicsNodes.size());
	for (auto curr : m_graphicsNodes) {
		nodeList.push_back(curr);
	}

	EngineContext engineContext(1, 1);
	for (auto& node : nodeList) {
		if (auto graphicsNode = dynamic_cast<GraphicsNode*>(node.get())) {
			graphicsNode->Initialize(engineContext);
		}
	}

	m_pipeline.CreateFromNodesList(nodeList);

	DumpPipelineGraph(m_pipeline, "pipeline_graph.dot");

	m_specialNodes = SelectSpecialNodes(m_pipeline);
}



std::vector<GraphicsNode*> GraphicsEngine::SelectSpecialNodes(Pipeline& pipeline) {
	std::vector<GraphicsNode*> specialNodes;

	for (NodeBase& node : pipeline) {
		// Pipeline disallows linking of its nodes, that's why it only returns const pointers.
		// We are not changing linking configuration here, so const_cast is justified.
		if (nodes::GetSceneByName* ptr = dynamic_cast<nodes::GetSceneByName*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetCameraByName* ptr = dynamic_cast<nodes::GetCameraByName*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetBackBuffer* ptr = dynamic_cast<nodes::GetBackBuffer*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetTime* ptr = dynamic_cast<nodes::GetTime*>(&node)) {
			specialNodes.push_back(ptr);
		}
		else if (nodes::GetEnvVariable* ptr = dynamic_cast<nodes::GetEnvVariable*>(&node)) {
			specialNodes.push_back(ptr);
		}
	}

	return specialNodes;
}


void GraphicsEngine::UpdateSpecialNodes() {
	std::vector<const Scene*> scenes;
	for (auto scene : m_scenes) {
		scenes.push_back(scene);
	}

	std::vector<const BasicCamera*> cameras;
	for (auto camera : m_cameras) {
		cameras.push_back(camera);
	}

	int backBufferIndex = m_swapChain->GetCurrentBufferIndex();
	Texture2D backBuffer = m_backBufferHeap->GetBackBuffer(backBufferIndex).GetResource();

	for (auto node : m_specialNodes) {
		if (auto* getScene = dynamic_cast<nodes::GetSceneByName*>(node)) {
			getScene->SetSceneList(scenes);
		}
		else if (auto* getCamera = dynamic_cast<nodes::GetCameraByName*>(node)) {
			getCamera->SetCameraList(cameras);
		}
		else if (auto* getBB = dynamic_cast<nodes::GetBackBuffer*>(node)) {
			getBB->SetBuffer(backBuffer);
		}
		else if (auto* getTime = dynamic_cast<nodes::GetTime*>(node)) {
			getTime->SetAbsoluteTime(m_absoluteTime.count() / 1e9);
		}
		else if (auto* getEnv = dynamic_cast<nodes::GetEnvVariable*>(node)) {
			getEnv->SetEnvVariableList(&m_envVariables);
		}
	}
}


std::string TidyTypeName(std::string name) {
	std::string s2;

	int inTemplate = 0;
	for (auto c : name) {
		if (c == '<') {
			inTemplate++;
			s2 += "&lt;";
		}
		else if (c == '>') {
			inTemplate--;
			s2 += "&gt;";
		}
		else {
			s2 += c;
		}
	}

	std::regex classFilter(R"(\s*class\s*)");
	s2 = std::regex_replace(s2, classFilter, "");

	std::regex structFilter(R"(\s*struct\s*)");
	s2 = std::regex_replace(s2, structFilter, "");

	std::regex enumFilter(R"(\s*enum\s*)");
	s2 = std::regex_replace(s2, enumFilter, "");

	std::regex ptrFilter(R"(\s*__ptr64\s*)");
	s2 = std::regex_replace(s2, ptrFilter, "");

	std::regex constFilter(R"(\s*const\s*)");
	s2 = std::regex_replace(s2, constFilter, "");

	std::regex namespaceFilter1(R"(\s*inl::gxeng::\s*)");
	s2 = std::regex_replace(s2, namespaceFilter1, "");

	std::regex namespaceFilter2(R"(\s*inl::\s*)");
	s2 = std::regex_replace(s2, namespaceFilter2, "");

	std::regex stringFilter(R"(\s*std::basic_string.*)");
	s2 = std::regex_replace(s2, stringFilter, "std::string");


	return s2;
}


void GraphicsEngine::DumpPipelineGraph(const Pipeline& pipeline, std::string file) {
	std::stringstream dot; // graphviz dot file

	struct PortMap {
		const NodeBase* parent;
		int portIndex;
	};

	std::map<const InputPortBase*, PortMap> inputParents;
	std::map<const OutputPortBase*, PortMap> outputParents;
	std::map<const NodeBase*, int> nodeIndexMap;

	// Fill node map and parent maps
	for (const NodeBase& node : pipeline) {
		int nodeIndex = nodeIndexMap.size();
		nodeIndexMap.insert({ &node, nodeIndex });

		for (int i = 0; i < node.GetNumInputs(); ++i) {
			inputParents.insert({ node.GetInput(i), PortMap{&node, i} });
		}
		for (int i = 0; i < node.GetNumOutputs(); ++i) {
			outputParents.insert({ node.GetOutput(i), PortMap{ &node, i } });
		}
	}

	// Write out preamble
	dot << "digraph structs {" << std::endl;
	dot << "rankdir=LR;" << std::endl;
	dot << "ranksep=\"0.8\";" << std::endl;
	dot << "node [shape=record];" << std::endl;
	dot << std::endl;

	// Write out nodes
	for (const auto& v : nodeIndexMap) {
		dot << "node" << v.second << " [shape=record, label=\"";
		dot << "&lt;&lt;&lt; " << TidyTypeName(typeid(*v.first).name()) << " &gt;&gt;&gt;";
		dot << " | {";
		// Inputs
		dot << "{";
		for (int i = 0; i < v.first->GetNumInputs(); ++i) {
			const InputPortBase* port = v.first->GetInput(i);
			dot << "<in" << i << "> ";
			dot << TidyTypeName(port->GetType().name());
			if (i < (int)v.first->GetNumInputs() - 1) {
				dot << " | ";
			}
		}
		dot << "} | ";
		// Outputs
		dot << "{";
		for (int i = 0; i < v.first->GetNumOutputs(); ++i) {
			const OutputPortBase* port = v.first->GetOutput(i);
			dot << "<out" << i << "> ";
			dot << TidyTypeName(port->GetType().name());
			if (i < (int)v.first->GetNumOutputs() - 1) {
				dot << " | ";
			}
		}
		dot << "}";

		dot << "}\"];" << std::endl;
	}

	dot << std::endl;

	// Write out links
	for (const auto& v : nodeIndexMap) {
		// Inputs
		for (int i = 0; i < v.first->GetNumInputs(); ++i) {
			const InputPortBase* target = v.first->GetInput(i);
			OutputPortBase* source = target->GetLink();
			if (source && outputParents.count(source) > 0) {
				auto srcNode = outputParents[source];
				auto tarNode = inputParents[target];

				dot << "node" << nodeIndexMap[srcNode.parent] << ":";
				dot << "out" << srcNode.portIndex;
				dot << " -> ";
				dot << "node" << nodeIndexMap[tarNode.parent] << ":";
				dot << "in" << tarNode.portIndex;
				dot << ";" << std::endl;
				assert(tarNode.portIndex == i);
			}
		}
	}

	dot << std::endl;

	// Write ranks
	/*
	std::map<NodeBase*, int> nodeRankMap;
	lemon::Bfs<lemon::ListDigraph> bfs{ pipeline.GetDependencyGraph() };
	bfs.init();
	for (lemon::ListDigraph::NodeIt it(pipeline.GetDependencyGraph()); it != lemon::INVALID; ++it) {
		if (lemon::countInArcs(pipeline.GetDependencyGraph(), it) == 0) {
			bfs.run(it);

			for (lemon::ListDigraph::NodeIt nit(pipeline.GetDependencyGraph()); nit != lemon::INVALID; ++nit) {
				auto* node = pipeline.GetNodeMap()[nit].get();
				int currentDist = bfs.reached(nit) ? bfs.dist(nit) : -1;
				int oldDist = nodeRankMap.count(node) > 0 ? nodeRankMap[node] : -1;
				nodeRankMap[node] =  std::max(currentDist, oldDist);
			}
		}
	}

	std::vector<std::pair<NodeBase*, int>> nodeRanks;
	for (auto& v : nodeRankMap) {
		nodeRanks.push_back({ v.first, v.second });
	}

	std::sort(nodeRanks.begin(), nodeRanks.end(), [](const auto& a, const auto& b) {
		return a.second < b.second;
	});

	int prevRank = -5;
	for (int i = 0; i < nodeRanks.size(); ++i) {
		if (prevRank != nodeRanks[i].second) {
			if (prevRank != -5) {
				dot << "}" << std::endl;
			}
			dot << "{rank=same;";
		}
		dot << " node" << nodeIndexMap[nodeRanks[i].first];
		prevRank = nodeRanks[i].second;
	}
	if (nodeRanks.size()) {
		dot << "}" << std::endl;
	}
	*/

	// Write closing
	dot << "}" << std::endl;


	// Write out file
	std::ofstream f(file, std::ios::trunc);
	if (f.is_open()) {
		f << dot.str();
	}
}



} // namespace gxeng
} // namespace inl