#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl::gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Resource : public gxapi::IResource {
public:
	Resource(ComPtr<ID3D12Resource>& native, std::nullptr_t);
	// passing device only because renderdoc crashes... fuck that... use GetDevice instead
	Resource(ComPtr<ID3D12Resource>& native, ComPtr<ID3D12Device> device);

	ID3D12Resource* GetNative();
	const ID3D12Resource* GetNative() const;

	gxapi::ResourceDesc GetDesc() const override;
	void* Map(unsigned subresourceIndex, const gxapi::MemoryRange* readRange = nullptr) override;
	void Unmap(unsigned subresourceIndex, const gxapi::MemoryRange* writtenRange = nullptr) override;
	void* GetGPUAddress() const override;

	unsigned GetNumMipLevels() const override;
	unsigned GetNumTexturePlanes() const override;
	unsigned GetNumArrayLevels() const override;
	unsigned GetNumSubresources() const override;
	unsigned GetSubresourceIndex(unsigned mipIdx, unsigned arrayIdx, unsigned planeIdx) const override;
	Vec3u64 GetSize(unsigned mipLevel = 0) const override;

	void SetName(const char* name) override;
private:
	ComPtr<ID3D12Resource> m_native;
	unsigned m_numMipLevels, m_numTexturePlanes, m_numArrayLevels;
};


} // namespace gxapi_dx12
