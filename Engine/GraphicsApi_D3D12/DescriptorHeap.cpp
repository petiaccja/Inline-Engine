#include "DescriptorHeap.hpp"

#include "NativeCast.hpp"
#include "d3dx12.h"

#include <cassert>
#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


DescriptorHeap::DescriptorHeap(ComPtr<ID3D12DescriptorHeap>& native)
	: m_native{native} {
}


gxapi::DescriptorHandle DescriptorHeap::At(size_t index) const {
	ID3D12Device* device;
	if (FAILED(m_native->GetDevice(IID_PPV_ARGS(&device)))) {
		throw std::runtime_error{"Could not get device for heap."};
	}

	size_t incrementSize = device->GetDescriptorHandleIncrementSize(m_native->GetDesc().Type);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleHelper{m_native->GetCPUDescriptorHandleForHeapStart(), (int)index, (unsigned)incrementSize};
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleHelper{m_native->GetGPUDescriptorHandleForHeapStart(), (int)index, (unsigned)incrementSize};

	gxapi::DescriptorHandle result;
	result.cpuAddress = reinterpret_cast<void*>(uintptr_t(cpuHandleHelper.ptr));
	result.gpuAddress = reinterpret_cast<void*>(uintptr_t(gpuHandleHelper.ptr));

	return result;
}

gxapi::DescriptorHeapDesc DescriptorHeap::GetDesc() const {
	return native_cast(m_native->GetDesc());
}


} // namespace gxapi_dx12
} // namespace inl
