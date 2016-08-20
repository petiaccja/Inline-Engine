#pragma once

#include "../GraphicsApi_LL/ICommandQueue.hpp"
#include "CommandQueue.hpp"
#include "CommandListTasks.hpp"
#include <queue>
#include <BaseLibrary/Logging/LogStream.hpp>


namespace inl {
namespace gxeng {

class CommandAllocatorPool;



struct FrameContext {
	float frameTime;
	exc::LogStream* log;

	gxapi::IGraphicsApi* gxApi;
	CommandAllocatorPool* commandAllocatorPool;
	ScratchSpacePool* scratchSpacePool;

	CommandQueue* commandQueue;
	std::queue<InitTask>* initQueue;
	std::queue<CleanTask>* cleanQueue;
	std::mutex* initMutex;
	std::mutex* cleanMutex;
	std::condition_variable* initCv;
	std::condition_variable* cleanCv;
};



}
}
