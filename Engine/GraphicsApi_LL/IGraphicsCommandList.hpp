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
class IRootSignature;


class IGraphicsCommandList : public ICommandList {
public:
	virtual ~IGraphicsCommandList() = default;

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

	virtual void ExecuteBundle(IGraphicsCommandList* bundle) = 0;

	// input assembler
	virtual void SetIndexBuffer(void* gpuVirtualAddress, size_t sizeInBytes, eFormat format) = 0;

	virtual void SetPrimitiveTopology(ePrimitiveTopology topology) = 0;

	virtual void SetVertexBuffers(unsigned startSlot,
								  unsigned count,
								  void** gpuVirtualAddress,
								  unsigned* sizeInBytes,
								  unsigned* strideInBytes) = 0;

	// output merger
	virtual void SetRenderTargets(unsigned numRenderTargets,
								  DescriptorHandle* renderTargets,
								  DescriptorHandle* depthStencil = nullptr) = 0;
	virtual void SetBlendFactor(float r, float g, float b, float a) = 0;
	virtual void SetStencilRef(unsigned stencilRef) = 0;

	// barriers
	// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions


	// rasterizer state
	virtual void SetScissorRects(unsigned numRects, Rectangle* rects) = 0;
	virtual void SetViewports(unsigned numViewports, Viewport* viewports) = 0;

	// set compute root signature stuff

	// set graphics root signature stuff
	virtual void SetGraphicsRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) = 0;
	virtual void SetGraphicsRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, uint32_t* value) = 0;
	virtual void SetGraphicsRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) = 0;
	virtual void SetGraphicsRootDescriptorTable(unsigned parameterIndex, DescriptorHandle baseHandle) = 0;
	virtual void SetGraphicsRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) = 0;

	virtual void SetGraphicsRootSignature(IRootSignature* rootSignature) = 0;

	// set pipeline state
	virtual void SetPipelineState(IPipelineState* pipelineState) = 0;


};


} // namespace gxapi
} // namespace inl