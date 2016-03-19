#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class GraphicsApi : public gxapi::IGraphicsApi {
public:

	// Command submission
	gxapi::ICommandQueue* CreateCommandQueue(gxapi::CommandQueueDesc desc) override;

	gxapi::ICommandAllocator* CreateCommandAllocator(gxapi::eCommandListType type) override;

	gxapi::IGraphicsCommandList* CreateGraphicsCommandList(gxapi::CommandListDesc desc) override;

	gxapi::ICopyCommandList* CreateCopyCommandList(gxapi::CommandListDesc desc) override;

	// Resources
	gxapi::IResource* CreateCommittedResource(gxapi::HeapProperties heapProperties,
	                                          gxapi::eHeapFlags heapFlags,
	                                          gxapi::ResourceDesc desc,
	                                          gxapi::eResourceState initialState,
	                                          gxapi::ClearValue* clearValue = nullptr) override;


	// Pipeline and binding
	gxapi::IRootSignature* CreateRootSignature(gxapi::RootSignatureDesc desc) override;

	gxapi::IPipelineState* CreateGraphicsPipelineState(gxapi::GraphicsPipelineStateDesc desc) override;

	gxapi::IDescriptorHeap* CreateDescriptorHeap(gxapi::DescriptorHeapDesc desc) override;

	void CreateConstantBufferView() override;
	void CreateDepthStencilView() override;
	void CreateRenderTargetView() override;
	void CreateShaderResourceView() override;
	

protected:
	ComPtr<ID3D12Device> m_device;
};


} // namespace gxapi_dx12
} // namespace inl
