#include "DescriptorHeap.hpp"
#include "../GraphicsApi_LL/Exception.hpp"

#include "NativeCast.hpp"
#include "d3dx12.h"

#include <cassert>
#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


DescriptorHeap::DescriptorHeap(ComPtr<ID3D12DescriptorHeap>& native)
	: m_native{ native }
{
	ID3D12Device* device;
	if (FAILED(m_native->GetDevice(IID_PPV_ARGS(&device)))) {
		throw inl::gxapi::Exception{ "Could not get device for heap." };
	}

	m_incrementSize = device->GetDescriptorHandleIncrementSize(m_native->GetDesc().Type);
	m_cpuBaseHandle = m_native->GetCPUDescriptorHandleForHeapStart();
	m_gpuBaseHandle = m_native->GetGPUDescriptorHandleForHeapStart();
}


gxapi::DescriptorHandle DescriptorHeap::At(size_t index) const {
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandleHelper{ m_cpuBaseHandle, (int)index, (unsigned)m_incrementSize };
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandleHelper{ m_gpuBaseHandle, (int)index, (unsigned)m_incrementSize };

	gxapi::DescriptorHandle result;
	result.cpuAddress = native_cast_ptr(cpuHandleHelper.ptr);
	result.gpuAddress = native_cast_ptr(gpuHandleHelper.ptr);

	return result;
}


gxapi::DescriptorHeapDesc DescriptorHeap::GetDesc() const {
	return native_cast(m_native->GetDesc());
}


uint32_t DescriptorHeap::GetIncrementSize() const {
	return m_incrementSize;
}


} // namespace gxapi_dx12
} // namespace inl
