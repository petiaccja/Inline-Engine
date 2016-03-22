#include "CommandQueue.hpp"

#include "NativeCast.hpp"

#include "CommandList.hpp"
#include "Fence.hpp"

#include <cassert>
#include <vector>

#include <d3d12.h>
#include "DisableWin32Macros.h"

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
	m_native->Signal(native_cast(fence), value); //TODO error check
}


void CommandQueue::Wait(gxapi::IFence* fence, uint64_t value) {
	m_native->Wait(native_cast(fence), value); //TODO error check
}

gxapi::CommandQueueDesc CommandQueue::GetDesc() const {
	return native_cast(m_native->GetDesc());
}


} // namespace gxapi_dx12
} // namespace inl
