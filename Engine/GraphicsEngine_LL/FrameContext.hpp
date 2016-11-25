#pragma once

#include <BaseLibrary/Logging/LogStream.hpp>
#include "../GraphicsApi_LL/ICommandQueue.hpp"

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
class Camera;
class BackBuffer;

struct FrameContext {
	std::chrono::nanoseconds frameTime;
	std::chrono::nanoseconds absoluteTime;
	exc::LogStream* log = nullptr;

	gxapi::IGraphicsApi* gxApi = nullptr;
	CommandAllocatorPool* commandAllocatorPool = nullptr;
	ScratchSpacePool* scratchSpacePool = nullptr;

	CommandQueue* commandQueue = nullptr;
	BackBuffer* backBuffer = nullptr;
	const std::set<Scene*>* scenes = nullptr;
	const std::set<Camera*>* cameras = nullptr;
	const std::vector<UploadManager::UploadDescription>* uploadRequests = nullptr;
	
	ResourceResidencyQueue* residencyQueue = nullptr;

	uint64_t frame;
};



}
}
