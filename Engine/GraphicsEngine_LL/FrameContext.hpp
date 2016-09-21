#pragma once

#include <BaseLibrary/Logging/LogStream.hpp>
#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include "CommandQueue.hpp"
#include "CommandListTasks.hpp"

#include <queue>
#include <chrono>
#include <set>
#include "ResourceResidencyQueue.hpp"


namespace inl {
namespace gxeng {


class CommandAllocatorPool;
class ScratchSpacePool;
class Scene;


struct FrameContext {
	std::chrono::nanoseconds frameTime;
	std::chrono::nanoseconds absoluteTime;
	exc::LogStream* log = nullptr;

	gxapi::IGraphicsApi* gxApi = nullptr;
	CommandAllocatorPool* commandAllocatorPool = nullptr;
	ScratchSpacePool* scratchSpacePool = nullptr;

	CommandQueue* commandQueue = nullptr;
	Texture2D* backBuffer = nullptr;
	const std::set<Scene*>* scenes = nullptr;
	
	ResourceResidencyQueue* residencyQueue = nullptr;

	uint64_t frame;
};



}
}
