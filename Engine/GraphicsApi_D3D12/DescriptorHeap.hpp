#pragma once

#include "../GraphicsApi_LL/IDescriptorHeap.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class DescriptorHeap : public gxapi::IDescriptorHeap {
public:
	DescriptorHeap(ComPtr<ID3D12DescriptorHeap>& native);
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;


	gxapi::DescriptorHandle At(size_t index) const override;

	size_t GetNumDescriptors() const override;
	gxapi::eDesriptorHeapType GetType() const override;
	bool IsShaderVisible() const override;

private:
	ComPtr<ID3D12DescriptorHeap> m_native;
};

}
}
