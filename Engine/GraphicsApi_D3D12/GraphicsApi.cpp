#include "GraphicsApi.hpp"

#include "CommandQueue.hpp"
#include "CommandAllocator.hpp"
#include "GraphicsCommandList.hpp"
#include "DescriptorHeap.hpp"
#include "NativeCast.hpp"

#include <stdexcept>
#include <cassert>

namespace inl {
namespace gxapi_dx12 {


gxapi::ICommandQueue* GraphicsApi::CreateCommandQueue(gxapi::CommandQueueDesc desc) {
	ComPtr<ID3D12CommandQueue> native;

	auto nativeDesc = native_cast(desc);
	if (FAILED(m_device->CreateCommandQueue(&nativeDesc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command queue.");
	}

	return new CommandQueue{native};
}


gxapi::ICommandAllocator* GraphicsApi::CreateCommandAllocator(gxapi::eCommandListType type) {
	ComPtr<ID3D12CommandAllocator> native;

	if (FAILED(m_device->CreateCommandAllocator(native_cast(type), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new CommandAllocator{native};
}


gxapi::IGraphicsCommandList* GraphicsApi::CreateGraphicsCommandList(gxapi::CommandListDesc desc) {
	ComPtr<ID3D12GraphicsCommandList> native;

	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, native_cast(desc.allocator), native_cast(desc.initialState), IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command list.");
	}

	return new GraphicsCommandList{native};
}


gxapi::ICopyCommandList* GraphicsApi::CreateCopyCommandList(gxapi::CommandListDesc desc) {
	//TODO not yet supported
	assert(false);
	return nullptr;
}


gxapi::IResource* GraphicsApi::CreateCommittedResource(gxapi::HeapProperties heapProperties,
	gxapi::eHeapFlags heapFlags,
	gxapi::ResourceDesc desc,
	gxapi::eResourceState initialState,
	gxapi::ClearValue* clearValue) {

	static_assert(false, "TODO");
	return nullptr;
}


gxapi::IRootSignature* GraphicsApi::CreateRootSignature() {
	assert(false);
	return nullptr;
}


gxapi::IPipelineState* GraphicsApi::CreateGraphicsPipelineState(gxapi::GraphicsPipelineStateDesc desc) {
	static_assert(false, "TODO");
	return nullptr;
}


gxapi::IDescriptorHeap* GraphicsApi::CreateDescriptorHeap(gxapi::DescriptorHeapDesc desc) {
	ComPtr<ID3D12DescriptorHeap> native;

	auto nativeDesc = native_cast(desc);
	if (FAILED(m_device->CreateDescriptorHeap(&nativeDesc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new DescriptorHeap{native};
}


void GraphicsApi::CreateConstantBufferView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateDepthStencilView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateRenderTargetView() {
	static_assert(false, "TODO");
}


void GraphicsApi::CreateShaderResourceView() {
	static_assert(false, "TODO");
}


} // namespace gxapi_dx12
} // namespace inl
