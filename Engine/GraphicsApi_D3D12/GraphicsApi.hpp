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
	gxapi::ICommandQueue* CreateCommandQueue() override;
	gxapi::ICommandAllocator* CreateCommandAllocator() override;
	gxapi::IGraphicsCommandList* CreateGraphicsCommandList() override;
	gxapi::ICopyCommandList* CreateCopyCommandList() override;

	// Resources
	gxapi::IResource* CreateCommittedResource() override;

	// Pipeline and binding
	gxapi::IRootSignature* CreateRootSignature() override;
	gxapi::IPipelineState* CreateGraphicsPipelineState() override;
	gxapi::IDescriptorHeap* CreateDescriptorHeap() override;

	void CreateConstantBufferView() override;
	void CreateDepthStencilView() override;
	void CreateRenderTargetView() override;
	void CreateShaderResourceView() override;

protected:
	ComPtr<ID3D12Device> m_device;
};


} // namespace gxapi_dx12
} // namespace inl
