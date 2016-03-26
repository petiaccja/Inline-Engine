#include "CommandAllocator.hpp"

#include "ExceptionExpansions.hpp"

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
	ThrowIfFailed(m_native->Reset());
}


} // namespace gxapi_dx12
} // namespace inl
