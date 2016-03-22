#include "Fence.hpp"

namespace inl {
namespace gxapi_dx12 {


Fence::Fence(ComPtr<ID3D12Fence>& native)
	: m_native(native) {
}


ID3D12Fence* Fence::GetNative() {
	return m_native.Get();
}


uint64_t Fence::Fetch() {
	return m_native->GetCompletedValue();
}


void Fence::Signal(uint64_t value) {
	m_native->Signal(value); //TODO error check
}


} // namespace gxapi_dx12
} // namespace inl
