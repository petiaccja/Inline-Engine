#include "CommandQueue.hpp"

#include "CommandList.hpp"
#include "Fence.hpp"

#include <vector>
#include <d3d12.h>

namespace inl {
namespace gxapi {


void CommandQueue::ExecuteCommandLists(uint32_t numCommandLists, ICommandList * const * commandLists)
{
	std::vector<ID3D12CommandList*> nativeCommandLists;
	nativeCommandLists.reserve(numCommandLists);

	for (int i = 0; i < numCommandLists; i++) {
		nativeCommandLists.push_back(static_cast<CommandList*>(commandLists[i])->GetNative());
	}

	m_native->ExecuteCommandLists(nativeCommandLists.size(), nativeCommandLists.data());
}

void CommandQueue::Signal(IFence* fence, uint64_t value)
{
	m_native->Signal(static_cast<Fence*>(fence)->GetNative(), value);
}

void CommandQueue::Wait(IFence* fence, uint64_t value)
{
	m_native->Wait(static_cast<Fence*>(fence)->GetNative(), value);
}

eCommandQueueType CommandQueue::GetType() const
{
	D3D12_COMMAND_LIST_TYPE nativeType = m_native->GetDesc().Type;
	switch (nativeType) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		return eCommandQueueType::GRAPHICS;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return eCommandQueueType::COMPUTE;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return eCommandQueueType::COPY;
	}

	//should not reach this point.
	//assert(false);

	return eCommandQueueType();
}

eCommandQueuePriority CommandQueue::GetPriority() const
{
	auto nativePriority = static_cast<D3D12_COMMAND_QUEUE_PRIORITY>(m_native->GetDesc().Priority);
	switch (nativePriority) {
	case D3D12_COMMAND_QUEUE_PRIORITY_NORMAL:
		return eCommandQueuePriority::NORMAL;
	case D3D12_COMMAND_QUEUE_PRIORITY_HIGH:
		return eCommandQueuePriority::HIGH;
	}

	//should not reach this point.
	//assert(false);

	return eCommandQueuePriority();
}

bool CommandQueue::IsGPUTimeoutEnabled() const
{
	bool isDisabled = (m_native->GetDesc().Flags & D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT) != 0;
	return !isDisabled;
}


}
}
