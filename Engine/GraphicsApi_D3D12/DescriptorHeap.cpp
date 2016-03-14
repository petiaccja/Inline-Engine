#include "DescriptorHeap.hpp"

#include <D3dx12.h>

#include <cassert>
#include <stdexcept>

namespace inl {
namespace gxapi_dx12 {


DescriptorHeap::DescriptorHeap(ID3D12DescriptorHeap* native) {
	if (native == nullptr) {
		throw std::runtime_error("Null pointer not allowed here.");
	}

	m_native = native;
}


DescriptorHeap::~DescriptorHeap() {
	m_native->Release();
}


gxapi::DescriptorHandle DescriptorHeap::At(size_t index) const {
	static_assert(false, "TODO find this helper struct");
	CD3DX12_CPU_DESCRIPTOR_HANDLE handleHelper{m_native->GetCPUDescriptorHandleForHeapStart()};

	return gxapi::DescriptorHandle();
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


}
}
