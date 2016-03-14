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


	virtual gxapi::DescriptorHandle At(size_t index) const override;

	virtual size_t GetNumDescriptors() const override;
	virtual gxapi::eDesriptorHeapType GetType() const override;
	virtual bool IsShaderVisible() const override;

private:
	ComPtr<ID3D12DescriptorHeap> m_native;
};

}
}
