#pragma once

#include "Common.hpp"
#include "ICommandList.hpp"


namespace inl {
namespace gxapi {


class ICommandQueue;
class ICommandAllocator;

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
	virtual IComputeCommandList* CreateComputeCommandList(CommandListDesc desc) = 0;
	virtual ICopyCommandList* CreateCopyCommandList(CommandListDesc desc) = 0;

	// Resources
	virtual IResource* CreateCommittedResource(HeapProperties heapProperties,
											   eHeapFlags heapFlags,
											   ResourceDesc desc,
											   eResourceState initialState,
											   ClearValue* clearValue = nullptr) = 0;

	// Pipeline and binding
	virtual IRootSignature* CreateRootSignature(RootSignatureDesc desc) = 0;
	virtual IPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;
	virtual IDescriptorHeap* CreateDescriptorHeap(DescriptorHeapDesc) = 0;

	// Views
	virtual void CreateConstantBufferView(ConstantBufferViewDesc desc, 
										  DescriptorHandle destination) = 0;

	virtual void CreateDepthStencilView(DepthStencilViewDesc desc,
										DescriptorHandle destination) = 0;
	virtual void CreateDepthStencilView(const IResource* resource,
										DescriptorHandle destination) = 0;

	virtual void CreateRenderTargetView(const IResource* resource,
										DescriptorHandle destination) = 0;
	virtual void CreateRenderTargetView(const IResource* resource,
										RenderTargetViewDesc desc,
										DescriptorHandle destination) = 0;

	virtual void CreateShaderResourceView(ShaderResourceViewDesc resource,
										  DescriptorHandle destination) = 0;
	virtual void CreateShaderResourceView(const IResource* resource,
										  DescriptorHandle destination) = 0;

	virtual void CreateUnorderedAccessView() = delete; // not needed yet

	virtual void CopyDescriptors(size_t numDstDescRanges, gxapi::DescriptorHandle* dstRangeStarts, uint32_t* dstRangeSizes, size_t numSrcDescRanges, gxapi::DescriptorHandle* srcRangeStarts, gxapi::eDesriptorHeapType descHeapsType) = 0;

	// Misc
	virtual IFence* CreateFence(uint64_t initialValue) = 0;
};


} // namespace gxapi
} // namespace inl