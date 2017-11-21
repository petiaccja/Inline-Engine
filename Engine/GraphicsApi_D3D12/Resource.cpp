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

Resource::Resource(ComPtr<ID3D12Resource>& native, std::nullptr_t) : Resource(native, ComPtr<ID3D12Device>(nullptr))
{}


Resource::Resource(ComPtr<ID3D12Resource>& native, ComPtr<ID3D12Device> device)
	: m_native{ native }
{
	auto desc = GetDesc();

	// calculate number of mip levels
	if (desc.type == gxapi::eResourceType::BUFFER) {
		m_numMipLevels = 1;
	}
	else {
		m_numMipLevels = desc.textureDesc.mipLevels;
	}

	// calculate number of texture planes
	if (desc.type == gxapi::eResourceType::BUFFER) {
		m_numTexturePlanes = 1;
	}
	else if (device.Get() != nullptr) {
		DXGI_FORMAT fmt = native_cast(desc.textureDesc.format);
		//ComPtr<ID3D12Device1> device;
		//HRESULT hr = m_native->GetDevice(IID_PPV_ARGS(&device));
		//assert(SUCCEEDED(hr));
		m_numTexturePlanes = D3D12GetFormatPlaneCount(device.Get(), fmt);
	}
	else {
		m_numTexturePlanes = 1;
	}

	// calculate number of array levels
	if (desc.type == gxapi::eResourceType::BUFFER) {
		m_numArrayLevels = 1;
	}
	else {
		m_numArrayLevels = desc.textureDesc.dimension == gxapi::eTextueDimension::THREE ? 1 : desc.textureDesc.depthOrArraySize;
	}
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


unsigned Resource::GetNumMipLevels() const {
	return m_numMipLevels;
}
unsigned Resource::GetNumTexturePlanes() const {
	return m_numTexturePlanes;
}
unsigned Resource::GetNumArrayLevels() const {
	return m_numArrayLevels;
}

unsigned Resource::GetNumSubresources() const {
	return m_numMipLevels * m_numTexturePlanes * m_numArrayLevels;
}
unsigned Resource::GetSubresourceIndex(unsigned mipIdx, unsigned arrayIdx, unsigned planeIdx) const {
	unsigned index = D3D12CalcSubresource(mipIdx, arrayIdx, planeIdx, GetNumMipLevels(), GetNumArrayLevels());
	assert(index < GetNumSubresources());
	return index;
}

Vec3u64 Resource::GetSize(int mipLevel) const {
	auto desc = GetDesc();

	if (mipLevel >= GetNumMipLevels()) {
		throw OutOfRangeException("Texture does not have that many mip levels.");
	}

	if (desc.type == gxapi::eResourceType::BUFFER) {
		return { desc.bufferDesc.sizeInBytes, 0, 0 };
	}

	Vec3u64 topLevelSize;
	switch (desc.textureDesc.dimension) {
		case gxapi::eTextueDimension::ONE:
			topLevelSize = { desc.textureDesc.width, 1, 1 };
			break;
		case gxapi::eTextueDimension::TWO:
			topLevelSize = { desc.textureDesc.width, desc.textureDesc.height, 1 };
			break;
		case gxapi::eTextueDimension::THREE:
			topLevelSize = { desc.textureDesc.width, desc.textureDesc.height, desc.textureDesc.depthOrArraySize };
			break;
	}

	for (int i = 1; i < mipLevel; ++i) {
		topLevelSize /= 2;
		topLevelSize = Vec3u64::Max(topLevelSize, { 1,1,1 });
	}

	return topLevelSize;
}


void Resource::SetName(const char* name) {
	size_t count = strlen(name);
	std::unique_ptr<wchar_t[]> dest = std::make_unique<wchar_t[]>(count + 1);
	mbstowcs(dest.get(), name, count);
	m_native->SetName(dest.get());
}


} // namespace gxapi_dx12
} // namespace inl
