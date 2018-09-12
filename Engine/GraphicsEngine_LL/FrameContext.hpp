#pragma once

#include "BasicCamera.hpp"
#include "CommandQueue.hpp"
#include "MemoryObject.hpp"
#include "UploadManager.hpp"

#include <BaseLibrary/Logging/LogStream.hpp>
#include <GraphicsApi_LL/ICommandQueue.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>

#include <chrono>
#include <queue>
#include <set>


namespace inl {
namespace gxeng {


	class CommandAllocatorPool;
	class CommandListPool;
	class ScratchSpacePool;
	class Scene;
	class BasicCamera;
	class PerspectiveCamera;
	class RenderTargetView2D;
	class ShaderManager;
	class MemoryManager;
	class ResourceResidencyQueue;
	class CbvSrvUavHeap;
	class RTVHeap;
	class DSVHeap;
	class CommandQueue;

	struct FrameContext {
		std::chrono::nanoseconds frameTime;
		std::chrono::nanoseconds absoluteTime;
		LogStream* log = nullptr;

		gxapi::IGraphicsApi* gxApi = nullptr;
		CommandAllocatorPool* commandAllocatorPool = nullptr;
		CommandListPool* commandListPool = nullptr;
		ScratchSpacePool* scratchSpacePool = nullptr;
		MemoryManager* memoryManager = nullptr;
		CbvSrvUavHeap* textureSpace = nullptr;
		RTVHeap* rtvHeap = nullptr;
		DSVHeap* dsvHeap = nullptr;
		ShaderManager* shaderManager = nullptr;

		CommandQueue* commandQueue = nullptr;
		Texture2D backBuffer;
		const std::set<Scene*>* scenes = nullptr;
		const std::set<BasicCamera*>* cameras = nullptr;
		const std::vector<UploadManager::UploadDescription>* uploadRequests = nullptr;

		ResourceResidencyQueue* residencyQueue = nullptr;

		uint64_t frame;
	};



} // namespace gxeng
} // namespace inl
