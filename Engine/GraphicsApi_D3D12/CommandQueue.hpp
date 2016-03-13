#pragma once

#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi {

class CommandQueue : public ICommandQueue
{
public:
	virtual void ExecuteCommandLists(uint32_t numCommandLists, ICommandList* const* commandLists) override;

	virtual void Signal(IFence* fence, uint64_t value) override;
	virtual void Wait(IFence* fence, uint64_t value) override;

	virtual eCommandQueueType GetType() const override;
	virtual eCommandQueuePriority GetPriority() const override;
	virtual bool IsGPUTimeoutEnabled() const override;

private:
	ID3D12CommandQueue* m_native;
};

}
}
