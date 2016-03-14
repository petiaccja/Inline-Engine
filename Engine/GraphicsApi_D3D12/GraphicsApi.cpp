#include "GraphicsApi.hpp"

#include <stdexcept>
#include "CommandQueue.hpp"
#include "CommandAllocator.hpp"
#include "GraphicsCommandList.hpp"

namespace inl {
namespace gxapi_dx12 {


gxapi::ICommandQueue* GraphicsApi::CreateCommandQueue() {
	ID3D12CommandQueue* native;

	//TODO get descriptor from parameter list
	D3D12_COMMAND_QUEUE_DESC desc;
	desc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.NodeMask = 0;

	if (FAILED(m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command queue.");
	}

	return new CommandQueue{native};
}


gxapi::ICommandAllocator* GraphicsApi::CreateCommandAllocator() {
	ID3D12CommandAllocator* native;

	//TODO get type from parameter list
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&native)))) {
		throw std::runtime_error("Could not create command allocator.");
	}

	return new CommandAllocator{native};
}


gxapi::IGraphicsCommandList* GraphicsApi::CreateGraphicsCommandList() {
	ID3D12GraphicsCommandList* native;

	static_assert(false, "TODO: graphics api functions need parameters.");
	//m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, );

	return new GraphicsCommandList{native};
}


gxapi::ICopyCommandList* GraphicsApi::CreateCopyCommandList()
{
	return nullptr;
}


gxapi::IResource* GraphicsApi::CreateCommittedResource()
{
	return nullptr;
}


gxapi::IRootSignature* GraphicsApi::CreateRootSignature()
{
	return nullptr;
}


gxapi::IPipelineState* GraphicsApi::CreateGraphicsPipelineState()
{
	return nullptr;
}


gxapi::IDescriptorHeap* GraphicsApi::CreateDescriptorHeap()
{
	return nullptr;
}


void GraphicsApi::CreateConstantBufferView()
{
}


void GraphicsApi::CreateDepthStencilView()
{
}


void GraphicsApi::CreateRenderTargetView()
{
}


void GraphicsApi::CreateShaderResourceView() {

}


}
}