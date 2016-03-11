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


class IGraphicsApi {
public:
	virtual ~IGraphicsApi() = default;
	

	// Command submission
	ICommandQueue* CreateCommandQueue();
	ICommandAllocator* CreateCommandAllocator();
	IGraphicsCommandList* CreateGraphicsCommandList();
	ICopyCommandList* CreateCopyCommandList();

	// Resources
	IResource* CreateCommittedResource();

	// Pipeline and binding
	IRootSignature* CreateRootSignature();
	IPipelineState* CreateGraphicsPipelineState();
	IDescriptorHeap* CreateDescriptorHeap();

	void CreateConstantBufferView();
	void CreateDepthStencilView();
	void CreateRenderTargetView();
	void CreateShaderResourceView();
	void CreateUnorderedAccessView() = delete;
};


} // namespace gxapi
} // namespace inl