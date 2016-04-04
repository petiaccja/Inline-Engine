#pragma once

#include "Common.hpp"


namespace inl {
namespace gxapi {


class ICommandQueue;
class ICommandAllocator;
class IGraphicsCommandList;
class ICopyCommandList;

class IFence;

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
	virtual IRootSignature* CreateRootSignature(RootSignatureDesc desc) = 0;
	virtual IPipelineState* CreateGraphicsPipelineState(GraphicsPipelineStateDesc desc) = 0;
	virtual IDescriptorHeap* CreateDescriptorHeap(DescriptorHeapDesc) = 0;

	// Views
	virtual void CreateConstantBufferView(ConstantBufferViewDesc desc, 
										  DescriptorHandle destination) = 0;

	virtual void CreateDepthStencilView(DepthStencilViewDesc desc,
										DescriptorHandle destination) = 0;
	virtual void CreateDepthStencilView(IResource* resource,
										DescriptorHandle destination) = 0;

	virtual void CreateRenderTargetView(RenderTargetViewDesc resource,
										DescriptorHandle destination) = 0;
	virtual void CreateRenderTargetView(IResource* resource,
										DescriptorHandle destination) = 0;

	virtual void CreateShaderResourceView(ShaderResourceViewDesc resource,
										  DescriptorHandle destination) = 0;
	virtual void CreateShaderResourceView(IResource* resource,
										  DescriptorHandle destination) = 0;

	virtual void CreateUnorderedAccessView() = delete; // not needed yet

	// Misc
	virtual IFence* CreateFence(uint64_t initialValue) = 0;
};


} // namespace gxapi
} // namespace inl