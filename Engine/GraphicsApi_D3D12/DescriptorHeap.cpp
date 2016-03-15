#include "DescriptorHeap.hpp"

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

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleHelper{m_native->GetCPUDescriptorHandleForHeapStart(), index, incrementSize};
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleHelper{m_native->GetGPUDescriptorHandleForHeapStart(), index, incrementSize};

	gxapi::DescriptorHandle result;
	result.cpuAddress = reinterpret_cast<void*>(uintptr_t(cpuHandleHelper.ptr));
	result.gpuAddress = reinterpret_cast<void*>(uintptr_t(gpuHandleHelper.ptr));

	return result;
}


size_t DescriptorHeap::GetNumDescriptors() const {
	return size_t();
}


gxapi::eDesriptorHeapType DescriptorHeap::GetType() const {
	using gxapi::eDesriptorHeapType;

	switch (m_native->GetDesc().Type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return eDesriptorHeapType::CBV_SRV_UAV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		return eDesriptorHeapType::SAMPLER;
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return eDesriptorHeapType::RTV;
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return eDesriptorHeapType::DSV;
	default:
		assert(false);
	}

	return eDesriptorHeapType{};
}


bool DescriptorHeap::IsShaderVisible() const {
	return (m_native->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) != 0;
}


} // namespace gxapi_dx12
} // namespace inl
