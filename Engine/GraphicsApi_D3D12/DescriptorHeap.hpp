#pragma once

#include "../GraphicsApi_LL/IDescriptorHeap.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../GraphicsApi_LL/DisableWin32Macros.h"

#include <d3d12.h>
#include <wrl.h>

namespace inl::gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class DescriptorHeap : public gxapi::IDescriptorHeap {
public:
	DescriptorHeap(ComPtr<ID3D12DescriptorHeap>& native, ID3D12Device* device /* REMOVE THAT FUCKING PARAMETER, FUCKING RENDERDOC */);
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;

	gxapi::DescriptorHandle At(size_t index) const override;

	gxapi::DescriptorHeapDesc GetDesc() const override;
	uint32_t GetIncrementSize() const override;

	ID3D12DescriptorHeap* GetNative();

private:
	ComPtr<ID3D12DescriptorHeap> m_native;
	uint32_t m_incrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuBaseHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuBaseHandle;
};


} // namespace inl::gxapi_dx12
