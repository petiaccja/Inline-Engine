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
		: m_commandQueue(graphicsApi->CreateCommandQueue(gxapi::CommandQueueDesc{type, priority})), m_progressFence(graphicsApi->CreateFence(0))
	{}
	CommandQueue(gxapi::ICommandQueue* queue, gxapi::IFence* fence)
		: m_commandQueue(queue), m_progressFence(fence)
	{}
	
	// General
	gxapi::ICommandQueue* GetUnderlyingQueue() {
		return m_commandQueue.get();
	}

	const gxapi::ICommandQueue* GetUnderlyingQueue() const {
		return m_commandQueue.get();
	}


	// Synchronization stuff
	std::pair<const gxapi::IFence*, uint64_t> Signal() {
		++m_fenceValue;
		m_commandQueue->Signal(m_progressFence.get(), m_fenceValue);
		return{ m_progressFence.get(), m_fenceValue };
	}

	void Wait(gxapi::IFence* fence, uint64_t value) {
		return m_commandQueue->Wait(fence, value);
	}


	// Queue stuff
	void ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const* commandLists) {
		m_commandQueue->ExecuteCommandLists(numCommandLists, commandLists);
	}

	gxapi::CommandQueueDesc GetDesc() const {
		return m_commandQueue->GetDesc();
	}
private:
	std::unique_ptr<gxapi::ICommandQueue> m_commandQueue;
	std::unique_ptr<gxapi::IFence> m_progressFence;
	unsigned long long m_fenceValue = 0;
};


}
}