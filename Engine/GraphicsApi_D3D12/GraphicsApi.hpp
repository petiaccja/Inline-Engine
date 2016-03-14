#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"

#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class GraphicsApi : public gxapi::IGraphicsApi {
public:


	// Command submission
	virtual gxapi::ICommandQueue* CreateCommandQueue() override;
	virtual gxapi::ICommandAllocator* CreateCommandAllocator() override;
	virtual gxapi::IGraphicsCommandList* CreateGraphicsCommandList() override;
	virtual gxapi::ICopyCommandList* CreateCopyCommandList() override;

	// Resources
	virtual gxapi::IResource* CreateCommittedResource() override;

	// Pipeline and binding
	virtual gxapi::IRootSignature* CreateRootSignature() override;
	virtual gxapi::IPipelineState* CreateGraphicsPipelineState() override;
	virtual gxapi::IDescriptorHeap* CreateDescriptorHeap() override;

	virtual void CreateConstantBufferView() override;
	virtual void CreateDepthStencilView() override;
	virtual void CreateRenderTargetView() override;
	virtual void CreateShaderResourceView() override;

protected:
	ComPtr<ID3D12Device> m_device;
};

}
}
