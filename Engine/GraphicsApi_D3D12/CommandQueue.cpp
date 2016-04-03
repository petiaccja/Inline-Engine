#include "CommandQueue.hpp"

#include "NativeCast.hpp"
#include "ExceptionExpansions.hpp"
#include "CommandList.hpp"
#include "Fence.hpp"

#include <cassert>
#include <vector>

#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {


CommandQueue::CommandQueue(ComPtr<ID3D12CommandQueue>& native)
	: m_native{native} {
}


ID3D12CommandQueue* CommandQueue::GetNative() {
	return m_native.Get();
}


void CommandQueue::ExecuteCommandLists(uint32_t numCommandLists, gxapi::ICommandList* const * commandLists) {
	std::vector<ID3D12CommandList*> nativeCommandLists;
	nativeCommandLists.reserve(numCommandLists);

	for (unsigned i = 0; i < numCommandLists; i++) {
		nativeCommandLists.push_back(static_cast<CommandList*>(commandLists[i])->GetNativeGenericList());
	}

	m_native->ExecuteCommandLists(nativeCommandLists.size(), nativeCommandLists.data());
}


void CommandQueue::Signal(gxapi::IFence* fence, uint64_t value) {
	ThrowIfFailed(m_native->Signal(native_cast(fence), value));
}


void CommandQueue::Wait(gxapi::IFence* fence, uint64_t value) {
	ThrowIfFailed(m_native->Wait(native_cast(fence), value));
}

gxapi::CommandQueueDesc CommandQueue::GetDesc() const {
	return native_cast(m_native->GetDesc());
}


} // namespace gxapi_dx12
} // namespace inl
