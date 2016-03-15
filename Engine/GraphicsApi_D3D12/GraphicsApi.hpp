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
	gxapi::ICommandQueue* CreateCommandQueue(gxapi::eCommandListType type,
		gxapi::eCommandQueuePriority priority,
		bool enableGpuTimeout) override;

	gxapi::ICommandAllocator* CreateCommandAllocator(gxapi::eCommandListType type) override;

	gxapi::IGraphicsCommandList* CreateGraphicsCommandList(gxapi::ICommandAllocator* allocator,
		gxapi::IPipelineState* initialState) override;

	gxapi::ICopyCommandList* CreateCopyCommandList(gxapi::ICommandAllocator* allocator,
		gxapi::IPipelineState* initialState) override;

	// Resources
	gxapi::IResource* CreateCommittedResource(/* long-ass parameter list */) override;


	// Pipeline and binding
	gxapi::IRootSignature* CreateRootSignature(/* long-ass complex shitstorm */) override;

	gxapi::IPipelineState* CreateGraphicsPipelineState(/* oh my fucking god */) override;

	gxapi::IDescriptorHeap* CreateDescriptorHeap(gxapi::eDesriptorHeapType type,
		size_t numDescriptors,
		bool isShaderVisible) override;

	void CreateConstantBufferView() override;
	void CreateDepthStencilView() override;
	void CreateRenderTargetView() override;
	void CreateShaderResourceView() override;
	

protected:
	ComPtr<ID3D12Device> m_device;
};


} // namespace gxapi_dx12
} // namespace inl
