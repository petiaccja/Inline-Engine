#pragma once

#include <BaseLibrary/Logging/LogStream.hpp>
#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include "BasicCamera.hpp"
#include "CommandQueue.hpp"

#include <queue>
#include <chrono>
#include <set>
#include "ResourceResidencyQueue.hpp"
#include "MemoryManager.hpp"


namespace inl {
namespace gxeng {


class CommandAllocatorPool;
class ScratchSpacePool;
class Scene;
class PerspectiveCamera;
class RenderTargetView2D;

struct FrameContext {
	std::chrono::nanoseconds frameTime;
	std::chrono::nanoseconds absoluteTime;
	exc::LogStream* log = nullptr;

	gxapi::IGraphicsApi* gxApi = nullptr;
	CommandAllocatorPool* commandAllocatorPool = nullptr;
	ScratchSpacePool* scratchSpacePool = nullptr;
	MemoryManager* memoryManager = nullptr;
	CbvSrvUavHeap* textureSpace = nullptr;
	RTVHeap* rtvHeap = nullptr;
	DSVHeap* dsvHeap = nullptr;
	ShaderManager* shaderManager = nullptr;

	CommandQueue* commandQueue = nullptr;
	RenderTargetView2D* backBuffer = nullptr;
	const std::set<Scene*>* scenes = nullptr;
	const std::set<BasicCamera*>* cameras = nullptr;
	const std::vector<UploadManager::UploadDescription>* uploadRequests = nullptr;
	
	ResourceResidencyQueue* residencyQueue = nullptr;

	uint64_t frame;
};



}
}
