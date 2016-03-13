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


void GraphicsCommandList::ClearRenderTarget(gxapi::DescriptorHandle rtv, ColorRGBA color, size_t numRects, Rectangle* rects) {
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView;
	renderTargetView.ptr = reinterpret_cast<size_t>(rtv.cpuAddress);

	float nativeColor[4] = {color.r, color.g, color.b, color.a};

	std::vector<D3D12_RECT> castedRects;
	castedRects.reserve(numRects);
	for (int i = 0; i < numRects; i++) {
		castedRects.push_back(native_cast(rects[i]));
	}

	m_native->ClearRenderTargetView(renderTargetView, nativeColor, castedRects.size(), castedRects.data());
}


void GraphicsCommandList::CopyBuffer(gxapi::IResource* dst, size_t dstOffset, gxapi::IResource* src, size_t srcOffset, size_t numBytes) {
	m_native->CopyBufferRegion(native_cast(dst), dstOffset, native_cast(src), srcOffset, numBytes);
}


void GraphicsCommandList::CopyResource(gxapi::IResource* dst, gxapi::IResource* src) {
	m_native->CopyResource(native_cast(dst), native_cast(src));
}


void GraphicsCommandList::CopyTexture(gxapi::IResource* dst, unsigned dstSubresourceIndex, gxapi::IResource* src, unsigned srcSubresourceIndex) {
	static_assert(false, "TODO");
	//m_native->CopyTextureRegion(native_cast(dst), )
}


void GraphicsCommandList::CopyTexture(gxapi::IResource* dst, TextureDescription dstDesc, gxapi::IResource* src, TextureDescription srcDesc, int offx, int offy, int offz, Cube region) {
	auto nativeDst = CreateTextureCopyLocation(dst, dstDesc);
	auto nativeSrc = CreateTextureCopyLocation(src, srcDesc);

	D3D12_BOX srcBox;
	srcBox.back = region.back;
	srcBox.front = region.front;
	srcBox.left = region.left;
	srcBox.right = region.right;
	srcBox.bottom = region.bottom;
	srcBox.top = region.top;

	m_native->CopyTextureRegion(&nativeDst, offx, offy, offz, &nativeSrc, &srcBox);
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


D3D12_TEXTURE_COPY_LOCATION GraphicsCommandList::CreateTextureCopyLocation(gxapi::IResource* texture, TextureDescription descrition) {

	D3D12_TEXTURE_COPY_LOCATION result;
	{
		result.pResource = native_cast(texture);
		result.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint;
		{
			D3D12_SUBRESOURCE_FOOTPRINT footprint;
			{
				footprint.Depth = descrition.depth;
				footprint.Format = native_cast(descrition.format);
				footprint.Height = descrition.height;
				footprint.Width = descrition.width;
				static_assert(false, "TODO");
				footprint.RowPitch;
			}
			placedFootprint.Footprint = footprint;
			placedFootprint.Offset = 0;
		}
		result.PlacedFootprint = placedFootprint;
	}

	return result;
}


}
}
