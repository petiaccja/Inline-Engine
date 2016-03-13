#include "GraphicsCommandList.hpp"

#include "PipelineState.hpp"
#include "NativeCast.hpp"

#include <vector>

namespace inl {
namespace gxapi_dx12 {


ID3D12CommandList* GraphicsCommandList::GetNativeGenericList() {
	return m_native;
}


void GraphicsCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_native->ClearState(native_cast(newState));
}


void GraphicsCommandList::Close() {
	m_native->Close();
}


void GraphicsCommandList::Reset(gxapi::ICommandAllocator * allocator, gxapi::IPipelineState * newState) {
	m_native->Reset(native_cast(allocator), native_cast(newState));
}


void GraphicsCommandList::ClearDepthStencil(gxapi::DescriptorHandle dsv, float depth, uint8_t stencil, size_t numRects, Rectangle* rects, bool clearDepth, bool clearStencil) {
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
	depthStencilView.ptr = reinterpret_cast<size_t>(dsv.cpuAddress);

	D3D12_CLEAR_FLAGS flags;
	flags = static_cast<D3D12_CLEAR_FLAGS>(clearDepth ? D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH : 0);
	flags = static_cast<D3D12_CLEAR_FLAGS>(flags | (clearStencil ? D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_STENCIL : 0));

	std::vector<D3D12_RECT> castedRects;
	castedRects.reserve(numRects);

	for (int i = 0; i < numRects; i++) {
		castedRects.push_back(native_cast(rects[i]));
	}

	m_native->ClearDepthStencilView(depthStencilView, flags, depth, stencil, castedRects.size(), castedRects.data());
}


void GraphicsCommandList::ClearRenderTarget(gxapi::DescriptorHandle rtv, ColorRGBA color, size_t numRects, Rectangle * rects) {
}


void GraphicsCommandList::CopyBuffer(gxapi::IResource * dst, size_t dstOffset, gxapi::IResource * src, size_t srcOffset, size_t numBytes) {
}


void GraphicsCommandList::CopyResource(gxapi::IResource * dst, gxapi::IResource * src) {
}


void GraphicsCommandList::CopyTexture(gxapi::IResource * dst, unsigned dstSubresourceIndex, gxapi::IResource * src, unsigned srcSubresourceIndex) {
}


void GraphicsCommandList::CopyTexture(gxapi::IResource * dst, TextureDescription dstDesc, gxapi::IResource * src, TextureDescription srcDesc, int offx, int offy, int offz, Cube region) {
}


void GraphicsCommandList::DrawIndexedInstanced(unsigned numIndices, unsigned startIndex, int vertexOffset, unsigned numInstances, unsigned startInstance) {
}


void GraphicsCommandList::DrawInstanced(unsigned numVertices, unsigned startVertex, unsigned numInstances, unsigned startInstance) {
}


void GraphicsCommandList::ExecuteBundle(ICommandList * bundle) {
}


void GraphicsCommandList::SetIndexBuffer(void * gpuVirtualAddress, size_t sizeInBytes, eFormat format) {
}


void GraphicsCommandList::SetPrimitiveTopology(ePrimitiveTopology topology) {
}


void GraphicsCommandList::SetVertexBuffers(unsigned startSlot, unsigned count, void ** gpuVirtualAddress, unsigned * sizeInBytes, unsigned * strideInBytes) {
}


void GraphicsCommandList::SetRenderTargets(unsigned numRenderTargets, gxapi::DescriptorHandle * renderTargets, gxapi::DescriptorHandle * depthStencil) {
}


void GraphicsCommandList::SetBlendFactor(float r, float g, float b, float a) {
}


void GraphicsCommandList::SetStencilRef(unsigned stencilRef) {
}


void GraphicsCommandList::SetPipelineState(gxapi::IPipelineState * pipelineState) {
}


}
}
