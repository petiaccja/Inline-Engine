#include "Resource.hpp"

#include "NativeCast.hpp"
#include "ExceptionExpansions.hpp"

namespace inl {
namespace gxapi_dx12 {

Resource::Resource(ComPtr<ID3D12Resource>& native)
	: m_native{native} {
}

ID3D12Resource* Resource::GetNative() {
	return m_native.Get();
}


gxapi::ResourceDesc Resource::GetDesc() const {
	return native_cast(m_native->GetDesc());
}


void* Resource::Map(unsigned subresourceIndex, const gxapi::MemoryRange* readRange) {
	void* result;

	D3D12_RANGE nativeRange;
	D3D12_RANGE* pNativeRange = nullptr;
	if (readRange != nullptr) {
		nativeRange = native_cast(*readRange);
		pNativeRange = &nativeRange;
	}

	ThrowIfFailed(m_native->Map(subresourceIndex, pNativeRange, &result));

	return result;
}


void Resource::Unmap(unsigned subresourceIndex, const gxapi::MemoryRange* writtenRange) {
	D3D12_RANGE nativeRange;
	D3D12_RANGE* pNativeRange = nullptr;
	if (writtenRange != nullptr) {
		nativeRange = native_cast(*writtenRange);
		pNativeRange = &nativeRange;
	}

	m_native->Unmap(subresourceIndex, pNativeRange);
}


void* Resource::GetGPUAddress() {
	return native_cast_ptr(m_native->GetGPUVirtualAddress());
}


} // namespace gxapi_dx12
} // namespace inl
