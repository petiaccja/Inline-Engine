#include "CommandAllocator.hpp"

#include "ExceptionExpansions.hpp"

#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


CommandAllocator::CommandAllocator(ComPtr<ID3D12CommandAllocator>& native, gxapi::eCommandListType type)
	: m_native{native}, m_type(type) {
}


ID3D12CommandAllocator* CommandAllocator::GetNative() {
	return m_native.Get();
}


void CommandAllocator::Reset() {
	ThrowIfFailed(m_native->Reset());
}


gxapi::eCommandListType CommandAllocator::GetType() const {
	return m_type;
}


} // namespace gxapi_dx12
} // namespace inl
