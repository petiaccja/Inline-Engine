#pragma once

#include <BaseLibrary/Logging/LogStream.hpp>
#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include "CommandQueue.hpp"
#include "CommandListTasks.hpp"

#include <queue>
#include <chrono>
#include <set>


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

	std::queue<InitTask>* initQueue = nullptr;
	std::queue<CleanTask>* cleanQueue = nullptr;
	std::mutex* initMutex = nullptr;
	std::mutex* cleanMutex = nullptr;
	std::condition_variable* initCv = nullptr;
	std::condition_variable* cleanCv = nullptr;
};



}
}
