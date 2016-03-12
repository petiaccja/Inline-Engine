#pragma once

#include "ICommandList.hpp"
#include "DescriptorHandle.hpp"

#include "Common.hpp"

#include <cstdint>


namespace inl {
namespace gxapi {

class IPipelineState;
class IResource;
class ICommandAllocator;


class IGraphicsCommandList : public ICommandList {
public:

	// Command list state
	virtual void ResetState(IPipelineState* newState = nullptr) = 0;
	virtual void Close() = 0;
	virtual void Reset(ICommandAllocator* allocator, IPipelineState* newState = nullptr) = 0;

	// Clear shit
	virtual void ClearDepthStencil(DescriptorHandle dsv,
								   float depth,
								   uint8_t stencil,
								   size_t numRects = 0,
								   Rectangle* rects = nullptr,
								   bool clearDepth = true,
								   bool clearStencil = false) = 0;

	virtual void ClearRenderTarget(DescriptorHandle rtv,
								   ColorRGBA color,
								   size_t numRects = 0,
								   Rectangle* rects = nullptr) = 0;


	// Resource copy
	virtual void CopyBuffer(IResource* dst, size_t dstOffset, IResource* src, size_t srcOffset, size_t numBytes) = 0;

	virtual void CopyResource(IResource* dst, IResource* src) = 0;

	virtual void CopyTexture(IResource* dst,
							 unsigned dstSubresourceIndex,
							 IResource* src,
							 unsigned srcSubresourceIndex) = 0;

	virtual void CopyTexture(IResource* dst,
							 TextureDescription dstDesc,
							 IResource* src,
							 TextureDescription srcDesc,
							 int offx, int offy, int offz,
							 Cube region) = 0;

	// Draw
	virtual void DrawIndexedInstanced(unsigned numIndices,
							  unsigned startIndex = 0,
							  int vertexOffset = 0,
							  unsigned numInstances = 1,
							  unsigned startInstance = 0) = 0;

	virtual void DrawInstanced(unsigned numVertices,
							   unsigned startVertex = 0,
							   unsigned numInstances = 1,
							   unsigned startInstance = 0) = 0;

	virtual void ExecuteBundle(ICommandList* bundle) = 0;

	// input assembler


	// output merger


	// rasterizer state

	// set compute root signature stuff

	// set graphics root signature stuff

	// set pipeline state
	virtual void SetPipelineState(IPipelineState* pipelineState) = 0;


};


}
}