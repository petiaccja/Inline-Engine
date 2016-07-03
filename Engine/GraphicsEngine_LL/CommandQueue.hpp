#pragma once


#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/ICommandQueue.hpp"
#include "../GraphicsApi_LL/IFence.hpp"


namespace inl {
namespace gxeng {


class CommandQueue {
public:
	// Consturctors
	CommandQueue(gxapi::IGraphicsApi* graphicsApi, gxapi::eCommandListType type, gxapi::eCommandQueuePriority priority = gxapi::eCommandQueuePriority::NORMAL)
		: m_commandQueue(graphicsApi->CreateCommandQueue(gxapi::CommandQueueDesc{type, priority})), m_fence(graphicsApi->CreateFence(0))
	{}
	CommandQueue(gxapi::ICommandQueue* queue, gxapi::IFence* fence)
		: m_commandQueue(queue), m_fence(fence)
	{}
	
	// General
	gxapi::ICommandQueue* GetUnderlyingQueue() {
		return m_commandQueue.get();
	}

	const gxapi::ICommandQueue* GetUnderlyingQueue() const {
		return m_commandQueue.get();
	}


	// Fence stuff
	gxapi::IFence* GetFence() {
		return m_fence.get();
	}

	const gxapi::IFence* GetFence() const {
		return m_fence.get();
	}

	unsigned long long GetFenceValue() const {
		return m_fenceValue;
	}

	unsigned long long IncrementFenceValue() {
		return ++m_fenceValue;
	}


	// Queue stuff
	void ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const* commandLists) {
		
	}

	void Signal(gxapi::IFence* fence, uint64_t value) {
		return m_commandQueue->Signal(fence, value);
	}
	void Wait(gxapi::IFence* fence, uint64_t value) {
		return m_commandQueue->Wait(fence, value);
	}

	gxapi::CommandQueueDesc GetDesc() const {
		return m_commandQueue->GetDesc();
	}
private:
	std::unique_ptr<gxapi::ICommandQueue> m_commandQueue;
	std::unique_ptr<gxapi::IFence> m_fence;
	unsigned long long m_fenceValue = 0;
};


}
}