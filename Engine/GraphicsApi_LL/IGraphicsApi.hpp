#pragma once


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
	virtual ICommandQueue* CreateCommandQueue() = 0;
	virtual ICommandAllocator* CreateCommandAllocator() = 0;
	virtual IGraphicsCommandList* CreateGraphicsCommandList() = 0;
	virtual ICopyCommandList* CreateCopyCommandList() = 0;

	// Resources
	virtual IResource* CreateCommittedResource() = 0;

	// Pipeline and binding
	virtual IRootSignature* CreateRootSignature() = 0;
	virtual IPipelineState* CreateGraphicsPipelineState() = 0;
	virtual IDescriptorHeap* CreateDescriptorHeap() = 0;

	virtual void CreateConstantBufferView() = 0;
	virtual void CreateDepthStencilView() = 0;
	virtual void CreateRenderTargetView() = 0;
	virtual void CreateShaderResourceView() = 0;
	virtual void CreateUnorderedAccessView() = delete; // not needed yet
};


} // namespace gxapi
} // namespace inl