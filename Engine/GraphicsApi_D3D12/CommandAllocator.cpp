#include "CommandAllocator.hpp"


#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


CommandAllocator::CommandAllocator(ComPtr<ID3D12CommandAllocator>& native)
	: m_native{native} {
}


ID3D12CommandAllocator* CommandAllocator::GetNative() {
	return m_native.Get();
}


void CommandAllocator::Reset() {
	m_native->Reset();
}


}
}
