#include "CommandQueue.hpp"

#include <d3d12.h>
#include <cassert>
#include <vector>

#include "NativeCast.hpp"

#include "CommandList.hpp"
#include "Fence.hpp"

namespace inl {
namespace gxapi_dx12 {


CommandQueue::CommandQueue(ComPtr<ID3D12CommandQueue>& native)
	: m_native{native} {
}


void CommandQueue::ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const * commandLists) {
	std::vector<ID3D12CommandList*> nativeCommandLists;
	nativeCommandLists.reserve(numCommandLists);

	for (int i = 0; i < numCommandLists; i++) {
		nativeCommandLists.push_back(static_cast<CommandList*>(commandLists[i])->GetNativeGenericList());
	}

	m_native->ExecuteCommandLists(nativeCommandLists.size(), nativeCommandLists.data());
}


void CommandQueue::Signal(gxapi::IFence* fence, uint64_t value) {
	m_native->Signal(native_cast(fence), value);
}


void CommandQueue::Wait(gxapi::IFence* fence, uint64_t value) {
	m_native->Wait(native_cast(fence), value);
}


gxapi::eCommandListType CommandQueue::GetType() const {
	return native_cast(m_native->GetDesc().Type);
}


gxapi::eCommandQueuePriority CommandQueue::GetPriority() const {
	return native_cast(static_cast<D3D12_COMMAND_QUEUE_PRIORITY>(m_native->GetDesc().Priority));
}


bool CommandQueue::IsGPUTimeoutEnabled() const {
	bool isDisabled = (m_native->GetDesc().Flags & D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT) != 0;
	return !isDisabled;
}


} // namespace gxapi_dx12
} // namespace inl
