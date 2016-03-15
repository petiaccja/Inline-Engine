#pragma once

#include "Common.hpp"


namespace inl {
namespace gxapi {


class ICommandQueue;
class ICommandAllocator;
class IGraphicsCommandList;
class ICopyCommandList;

class IResource;

class IRootSignature;
class IPipelineState;
class IDescriptorHeap;

// todo: descriptor view bullshit
class IGraphicsApi {
public:
	virtual ~IGraphicsApi() = default;
	

	// Command submission
	virtual ICommandQueue* CreateCommandQueue(CommandQueueDesc desc) = 0;

	virtual ICommandAllocator* CreateCommandAllocator(eCommandListType type) = 0;

	virtual IGraphicsCommandList* CreateGraphicsCommandList(CommandListDesc desc) = 0;

	virtual ICopyCommandList* CreateCopyCommandList(CommandListDesc desc) = 0;

	// Resources
	virtual IResource* CreateCommittedResource(HeapProperties heapProperties,
											   eHeapFlags heapFlags,
											   ResourceDesc desc,
											   eResourceState initialState,
											   ClearValue* clearValue = nullptr) = 0;


	// Pipeline and binding
	virtual IRootSignature* CreateRootSignature(/* long-ass complex shitstorm */) = 0;

	virtual IPipelineState* CreateGraphicsPipelineState(/* oh my fucking god */) = 0;

	virtual IDescriptorHeap* CreateDescriptorHeap(DescriptorHeapDesc) = 0;

	virtual void CreateConstantBufferView() = 0;
	virtual void CreateDepthStencilView() = 0;
	virtual void CreateRenderTargetView() = 0;
	virtual void CreateShaderResourceView() = 0;
	virtual void CreateUnorderedAccessView() = delete; // not needed yet
};


} // namespace gxapi
} // namespace inl