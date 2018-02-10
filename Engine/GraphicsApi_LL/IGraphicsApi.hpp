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

class ICapabilityQuery;


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
	virtual ICommandList* CreateCommandList(eCommandListType type, CommandListDesc desc) = 0;

	// Resources
	virtual IResource* CreateCommittedResource(HeapProperties heapProperties,
											   eHeapFlags heapFlags,
											   ResourceDesc desc,
											   eResourceState initialState,
											   ClearValue* clearValue = nullptr) = 0;

	// Pipeline and binding
	virtual IRootSignature* CreateRootSignature(RootSignatureDesc desc) = 0;
	virtual IPipelineState* CreateGraphicsPipelineState(const GraphicsPipelineStateDesc& desc) = 0;
	virtual IPipelineState* CreateComputePipelineState(const ComputePipelineStateDesc& desc) = 0;
	virtual IDescriptorHeap* CreateDescriptorHeap(DescriptorHeapDesc) = 0;

	// Views
	virtual void CreateConstantBufferView(ConstantBufferViewDesc desc,
										  DescriptorHandle destination) = 0;

	virtual void CreateDepthStencilView(DepthStencilViewDesc desc,
										DescriptorHandle destination) = 0;
	virtual void CreateDepthStencilView(const IResource* resource,
										DescriptorHandle destination) = 0;
	virtual void CreateDepthStencilView(const IResource * resource,
										DepthStencilViewDesc desc,
										DescriptorHandle destination) = 0;

	virtual void CreateRenderTargetView(const IResource* resource,
										DescriptorHandle destination) = 0;
	virtual void CreateRenderTargetView(const IResource* resource,
										RenderTargetViewDesc desc,
										DescriptorHandle destination) = 0;

	virtual void CreateShaderResourceView(ShaderResourceViewDesc descriptor,
										  DescriptorHandle destination) = 0;
	virtual void CreateShaderResourceView(const IResource* resource,
										  DescriptorHandle destination) = 0;
	virtual void CreateShaderResourceView(const IResource* resource,
										  ShaderResourceViewDesc descriptor,
										  DescriptorHandle destination) = 0;

	virtual void CreateUnorderedAccessView(UnorderedAccessViewDesc descriptor,
										   DescriptorHandle destination) = 0;
	virtual void CreateUnorderedAccessView(const IResource* resource,
										   DescriptorHandle destination) = 0;
	virtual void CreateUnorderedAccessView(const IResource* resource,
										   UnorderedAccessViewDesc descriptor,
										   DescriptorHandle destination) = 0;

	virtual void CreateUnorderedAccessView() = delete; // not needed yet

	virtual void CopyDescriptors(size_t numSrcDescRanges,
								 gxapi::DescriptorHandle* srcRangeStarts,
								 size_t numDstDescRanges,
								 gxapi::DescriptorHandle* dstRangeStarts,
								 uint32_t* rangeCounts,
								 gxapi::eDescriptorHeapType descHeapsType) = 0;

	virtual void CopyDescriptors(size_t numSrcDescRanges,
								 gxapi::DescriptorHandle* srcRangeStarts,
								 uint32_t* srcRangeLengths,
								 size_t numDstDescRanges,
								 gxapi::DescriptorHandle* dstRangeStarts,
								 uint32_t* dstRangeLengths,
								 gxapi::eDescriptorHeapType descHeapsType) = 0;

	virtual void CopyDescriptors(gxapi::DescriptorHandle srcStart,
								 gxapi::DescriptorHandle dstStart,
								 size_t rangeCount,
								 gxapi::eDescriptorHeapType descHeapsType) = 0;

	// Misc
	virtual IFence* CreateFence(uint64_t initialValue) = 0;

	virtual void MakeResident(const std::vector<gxapi::IResource*>& objects) = 0;
	virtual void Evict(const std::vector<gxapi::IResource*>& objects) = 0;

	// Debug
	virtual void ReportLiveObjects() const = 0;

	virtual ICapabilityQuery* GetCapabilityQuery() const = 0;
};


} // namespace gxapi
} // namespace inl