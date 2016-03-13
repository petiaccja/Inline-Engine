#pragma once

#include "../GraphicsApi_LL/IGraphicsCommandList.hpp"
#include "CommandList.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

class GraphicsCommandList : public CommandList, public gxapi::IGraphicsCommandList {
public:

	virtual ID3D12CommandList* GetNativeGenericList() override;



	// Command list state
	virtual void ResetState(gxapi::IPipelineState* newState = nullptr) override;
	virtual void Close() override;
	virtual void Reset(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* newState = nullptr) override;

	// Clear shit
	virtual void ClearDepthStencil(gxapi::DescriptorHandle dsv,
		float depth,
		uint8_t stencil,
		size_t numRects = 0,
		Rectangle* rects = nullptr,
		bool clearDepth = true,
		bool clearStencil = false) override;

	virtual void ClearRenderTarget(gxapi::DescriptorHandle rtv,
		ColorRGBA color,
		size_t numRects = 0,
		Rectangle* rects = nullptr) override;


	// Resource copy
	virtual void CopyBuffer(gxapi::IResource* dst, size_t dstOffset, gxapi::IResource* src, size_t srcOffset, size_t numBytes) override;

	virtual void CopyResource(gxapi::IResource* dst, gxapi::IResource* src) override;

	virtual void CopyTexture(gxapi::IResource* dst,
		unsigned dstSubresourceIndex,
		gxapi::IResource* src,
		unsigned srcSubresourceIndex) override;

	virtual void CopyTexture(gxapi::IResource* dst,
		TextureDescription dstDesc,
		gxapi::IResource* src,
		TextureDescription srcDesc,
		int offx, int offy, int offz,
		Cube region) override;

	// Draw
	virtual void DrawIndexedInstanced(unsigned numIndices,
		unsigned startIndex = 0,
		int vertexOffset = 0,
		unsigned numInstances = 1,
		unsigned startInstance = 0) override;

	virtual void DrawInstanced(unsigned numVertices,
		unsigned startVertex = 0,
		unsigned numInstances = 1,
		unsigned startInstance = 0) override;

	virtual void ExecuteBundle(ICommandList* bundle) override;

	// input assembler
	virtual void SetIndexBuffer(void* gpuVirtualAddress, size_t sizeInBytes, eFormat format) override;

	virtual void SetPrimitiveTopology(ePrimitiveTopology topology) override;

	virtual void SetVertexBuffers(unsigned startSlot,
		unsigned count,
		void** gpuVirtualAddress,
		unsigned* sizeInBytes,
		unsigned* strideInBytes) override;

	// output merger
	virtual void SetRenderTargets(unsigned numRenderTargets,
		gxapi::DescriptorHandle* renderTargets,
		gxapi::DescriptorHandle* depthStencil = nullptr) override;
	virtual void SetBlendFactor(float r, float g, float b, float a) override;
	virtual void SetStencilRef(unsigned stencilRef) override;

	// barriers

	// rasterizer state

	// set compute root signature stuff

	// set graphics root signature stuff

	// set pipeline state
	virtual void SetPipelineState(gxapi::IPipelineState* pipelineState) override;



protected:
	ID3D12GraphicsCommandList* m_native;

protected:
	static D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* texture, TextureDescription descrition);
	static D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* texture, unsigned subresourceIndex);
};

}
}
