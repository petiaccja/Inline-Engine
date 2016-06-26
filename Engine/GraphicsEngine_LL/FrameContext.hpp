#pragma once

#include "../GraphicsApi_LL/ICommandQueue.hpp"


namespace inl {
namespace gxeng {

class CommandAllocatorPool;


struct FrameContext {
	float frameTime;
	CommandAllocatorPool* commandAllocatorPool;
	gxapi::ICommandQueue* commandQueue;
};


}
}