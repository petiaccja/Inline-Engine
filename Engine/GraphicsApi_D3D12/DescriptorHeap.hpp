#pragma once

#include "../GraphicsApi_LL/IDescriptorHeap.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

class DescriptorHeap : public gxapi::IDescriptorHeap {
public:
	DescriptorHeap(ID3D12DescriptorHeap* native);
	~DescriptorHeap();
	DescriptorHeap(const DescriptorHeap&) = delete;
	DescriptorHeap& operator=(const DescriptorHeap&) = delete;


	virtual gxapi::DescriptorHandle At(size_t index) const override;

	virtual size_t GetNumDescriptors() const override;
	virtual gxapi::eDesriptorHeapType GetType() const override;
	virtual bool IsShaderVisible() const override;

private:
	ID3D12DescriptorHeap* m_native;
};

}
}
