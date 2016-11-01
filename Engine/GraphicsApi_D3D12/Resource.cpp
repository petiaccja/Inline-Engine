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

const ID3D12Resource* Resource::GetNative() const {
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


void* Resource::GetGPUAddress() const {
	return native_cast_ptr(m_native->GetGPUVirtualAddress());
}


void Resource::SetName(const char* name) {
	size_t count = strlen(name);
	std::unique_ptr<wchar_t[]> dest = std::make_unique<wchar_t[]>(count + 1);
#define _CRT_SECURE_NO_WARNINGS 1
	mbstowcs(dest.get(), name, count);
	m_native->SetName(dest.get());
}


} // namespace gxapi_dx12
} // namespace inl
