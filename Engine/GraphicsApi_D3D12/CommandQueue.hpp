#pragma once

#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

class CommandQueue : public gxapi::ICommandQueue {
public:
	CommandQueue(ID3D12CommandQueue* native);
	~CommandQueue();
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue& operator=(const CommandQueue&) = delete;

	virtual void ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const* commandLists) override;

	virtual void Signal(gxapi::IFence* fence, uint64_t value) override;
	virtual void Wait(gxapi::IFence* fence, uint64_t value) override;

	virtual gxapi::eCommandListType GetType() const override;
	virtual gxapi::eCommandQueuePriority GetPriority() const override;
	virtual bool IsGPUTimeoutEnabled() const override;

private:
	ID3D12CommandQueue* m_native;
};

}
}
