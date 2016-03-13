#include "CommandQueue.hpp"

#include <d3d12.h>
#include <cassert>
#include <vector>

#include "CommandList.hpp"
#include "Fence.hpp"

namespace inl {
namespace gxapi_dx12 {

void CommandQueue::ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList * const * commandLists) {
	std::vector<ID3D12CommandList*> nativeCommandLists;
	nativeCommandLists.reserve(numCommandLists);

	for (int i = 0; i < numCommandLists; i++) {
		nativeCommandLists.push_back(static_cast<CommandList*>(commandLists[i])->GetNative());
	}

	m_native->ExecuteCommandLists(nativeCommandLists.size(), nativeCommandLists.data());
}

void CommandQueue::Signal(gxapi::IFence* fence, uint64_t value) {
	m_native->Signal(static_cast<Fence*>(fence)->GetNative(), value);
}

void CommandQueue::Wait(gxapi::IFence* fence, uint64_t value) {
	m_native->Wait(static_cast<Fence*>(fence)->GetNative(), value);
}

gxapi::eCommandQueueType CommandQueue::GetType() const {
	D3D12_COMMAND_LIST_TYPE nativeType = m_native->GetDesc().Type;
	switch (nativeType) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
	case D3D12_COMMAND_LIST_TYPE_BUNDLE:
		return gxapi::eCommandQueueType::GRAPHICS;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return gxapi::eCommandQueueType::COMPUTE;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return gxapi::eCommandQueueType::COPY;
	default:
		assert(false);
	}

	return gxapi::eCommandQueueType();
}

gxapi::eCommandQueuePriority CommandQueue::GetPriority() const {
	auto nativePriority = static_cast<D3D12_COMMAND_QUEUE_PRIORITY>(m_native->GetDesc().Priority);
	switch (nativePriority) {
	case D3D12_COMMAND_QUEUE_PRIORITY_NORMAL:
		return gxapi::eCommandQueuePriority::NORMAL;
	case D3D12_COMMAND_QUEUE_PRIORITY_HIGH:
		return gxapi::eCommandQueuePriority::HIGH;
	default:
		assert(false);
	}

	return gxapi::eCommandQueuePriority();
}

bool CommandQueue::IsGPUTimeoutEnabled() const {
	bool isDisabled = (m_native->GetDesc().Flags & D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT) != 0;
	return !isDisabled;
}


}
}
