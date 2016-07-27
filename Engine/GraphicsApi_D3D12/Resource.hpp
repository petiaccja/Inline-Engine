#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class Resource : public gxapi::IResource {
public:
	Resource(ComPtr<ID3D12Resource>& native);

	ID3D12Resource* GetNative();
	const ID3D12Resource* GetNative() const;

	gxapi::ResourceDesc GetDesc() const override;
	void* Map(unsigned subresourceIndex, const gxapi::MemoryRange* readRange = nullptr) override;
	void Unmap(unsigned subresourceIndex, const gxapi::MemoryRange* writtenRange = nullptr) override;
	void* GetGPUAddress() const override;

private:
	ComPtr<ID3D12Resource> m_native;
};


} // namespace gxapi_dx12
} // namespace inl
