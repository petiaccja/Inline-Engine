#include "GraphicsApi.hpp"

#include "CommandQueue.hpp"
#include "CommandAllocator.hpp"
#include "GraphicsCommandList.hpp"
#include "NativeCast.hpp"

#include <stdexcept>
#include <cassert>

namespace inl {
namespace gxapi_dx12 {


gxapi::ICommandQueue* GraphicsApi::CreateCommandQueue(gxapi::eCommandListType type, gxapi::eCommandQueuePriority priority, bool enableGpuTimeout) {
	ComPtr<ID3D12CommandQueue> native;

	//TODO get descriptor from parameter list
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = native_cast(type);
	desc.Flags =  (enableGpuTimeout==false) ? D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT : D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = native_cast(priority);
	desc.NodeMask = 0;

	if (FAILED(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command queue.");
	}

	return new CommandQueue{native};
}


gxapi::ICommandAllocator* GraphicsApi::CreateCommandAllocator(gxapi::eCommandListType type) {
	ComPtr<ID3D12CommandAllocator> native;

	//TODO get type from parameter list
	if (FAILED(m_device->CreateCommandAllocator(native_cast(type), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new CommandAllocator{native};
}


gxapi::IGraphicsCommandList* GraphicsApi::CreateGraphicsCommandList(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* initialState) {
	ComPtr<ID3D12GraphicsCommandList> native;

	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, native_cast(allocator), native_cast(initialState), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command list.");
	}

	return new GraphicsCommandList{native};
}


gxapi::ICopyCommandList* GraphicsApi::CreateCopyCommandList(gxapi::ICommandAllocator * allocator, gxapi::IPipelineState * initialState) {
	//TODO not yet supported
	assert(false);
	return nullptr;
}


gxapi::IResource* GraphicsApi::CreateCommittedResource() {
	return nullptr;
}


gxapi::IRootSignature* GraphicsApi::CreateRootSignature() {
	return nullptr;
}


gxapi::IPipelineState* GraphicsApi::CreateGraphicsPipelineState() {
	return nullptr;
}


gxapi::IDescriptorHeap* GraphicsApi::CreateDescriptorHeap(gxapi::eDesriptorHeapType type, size_t numDescriptors, bool isShaderVisible) {
	return nullptr;
}


void GraphicsApi::CreateConstantBufferView() {
}


void GraphicsApi::CreateDepthStencilView() {
}


void GraphicsApi::CreateRenderTargetView() {
}


void GraphicsApi::CreateShaderResourceView() {
}


} // namespace gxapi_dx12
} // namespace inl
