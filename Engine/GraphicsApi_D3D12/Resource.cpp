#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Resource.hpp"

#include "NativeCast.hpp"
#include "ExceptionExpansions.hpp"

#include "D3dx12.h"

#include <cassert>


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


unsigned Resource::GetNumMipLevels() {
	auto desc = GetDesc();
	if (desc.type == gxapi::eResourceType::BUFFER) {
		return 0;
	}
	else {
		return desc.textureDesc.mipLevels;
	}
}
unsigned Resource::GetNumTexturePlanes() {
	auto desc = GetDesc();
	if (desc.type == gxapi::eResourceType::BUFFER) {
		return 0;
	}
	else {
		DXGI_FORMAT fmt = native_cast(desc.textureDesc.format);
		ID3D12Device1* device;
		HRESULT hr = m_native->GetDevice(IID_PPV_ARGS(&device));
		assert(SUCCEEDED(hr));
		D3D12GetFormatPlaneCount(device, fmt);
	}
}
unsigned Resource::GetNumArrayLevels() {
	auto desc = GetDesc();
	if (desc.type == gxapi::eResourceType::BUFFER) {
		return 0;
	}
	else {
		return desc.textureDesc.dimension == gxapi::eTextueDimension::THREE ? 1 : desc.textureDesc.depthOrArraySize;
	}
}


void Resource::SetName(const char* name) {
	size_t count = strlen(name);
	std::unique_ptr<wchar_t[]> dest = std::make_unique<wchar_t[]>(count + 1);
	mbstowcs(dest.get(), name, count);
	m_native->SetName(dest.get());
}


} // namespace gxapi_dx12
} // namespace inl
