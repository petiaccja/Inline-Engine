#pragma once

#include "GpuBuffer.hpp"
#include "DisposableConstBuffer.hpp"
#include "Binder.hpp"
#include "../GraphicsApi_LL/Common.hpp"
#include "../BaseLibrary/Range.hpp"
#include <mathfu/vector_2.h>


namespace inl {
namespace gxapi {
class ICommandAllocator;
class IPipelineState;
}
}

namespace inl {
namespace gxeng {

class CommandAllocatorPool;


struct SubTexture1D {
	SubTexture1D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 int firstPixel = -1,
				 int lastPixel = -1)
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		firstPixel(firstPixel),
		lastPixel(lastPixel) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	int firstPixel, lastPixel;
};

struct SubTexture2D {
	SubTexture2D(unsigned mipLevel = 0,
				 unsigned arrayIndex = 0,
				 mathfu::Vector<int, 2> corner1 = { -1, -1 },
				 mathfu::Vector<int, 2> corner2 = { -1, -1 })
		: mipLevel(mipLevel),
		arrayIndex(arrayIndex),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	unsigned arrayIndex;
	mathfu::Vector<int, 2> corner1;
	mathfu::Vector<int, 2> corner2;
};

struct SubTexture3D {
	SubTexture3D(unsigned mipLevel = 0,
				 mathfu::Vector<int, 3> corner1 = { -1, -1, -1 },
				 mathfu::Vector<int, 3> corner2 = { -1, -1, -1 })
		: mipLevel(mipLevel),
		corner1(corner1),
		corner2(corner2) {}

	unsigned mipLevel;
	mathfu::Vector<int, 3> corner1;
	mathfu::Vector<int, 3> corner2;
};



class CopyCommandList {
public:
	CopyCommandList(CommandAllocatorPool& cmdAllocatorPool);
protected:
	// Command list state
	virtual void ResetState(gxapi::IPipelineState* newState = nullptr) = 0;
	virtual void Close() = 0;
	virtual void Reset(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* newState = nullptr) = 0;

	// Clear shit
	virtual void ClearDepthStencil(Texture2D*,
								   float depth,
								   uint8_t stencil,
								   size_t numRects = 0,
								   gxapi::Rectangle* rects = nullptr,
								   bool clearDepth = true,
								   bool clearStencil = false) = 0;

	virtual void ClearRenderTarget(Texture2D*,
								   gxapi::ColorRGBA color,
								   size_t numRects = 0,
								   gxapi::Rectangle* rects = nullptr) = 0;


	// Resource copy
	virtual void CopyBuffer(GenericBuffer*, size_t dstOffset, GenericBuffer* src, size_t srcOffset, size_t numBytes) = 0;

	virtual void CopyResource(GenericBuffer* dst, GenericBuffer* src) = 0;

	virtual void CopyTexture(Texture1D* dst,
							 Texture1D* src,
							 SubTexture1D dstPlace = {},
							 SubTexture1D srcPlace = {}) = 0;
	virtual void CopyTexture(Texture2D* dst,
							 Texture2D* src,
							 SubTexture2D dstPlace = {},
							 SubTexture2D srcPlace = {}) = 0;
	virtual void CopyTexture(Texture3D* dst,
							 Texture3D* src,
							 SubTexture3D dstPlace = {},
							 SubTexture3D srcPlace = {}) = 0;

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

	//!!! virtual void ExecuteBundle(IGraphicsCommandList* bundle) = 0;

	// input assembler
	virtual void SetIndexBuffer(IndexBuffer*, bool is32Bit) = 0;

	virtual void SetPrimitiveTopology(gxapi::ePrimitiveTopology topology) = 0;

	virtual void SetVertexBuffers(unsigned startSlot,
								  unsigned count,
								  VertexBuffer**,
								  unsigned* sizeInBytes,
								  unsigned* strideInBytes) = 0;

	// output merger
	virtual void SetRenderTargets(unsigned numRenderTargets,
								  Texture2D** renderTargets,
								  Texture2D* depthStencil = nullptr) = 0;
	virtual void SetBlendFactor(float r, float g, float b, float a) = 0;
	virtual void SetStencilRef(unsigned stencilRef) = 0;

	// barriers
	// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions
	virtual void ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) = 0;

	template <class... Barriers>
	void ResourceBarrier(Barriers&&... barriers);

	// rasterizer state
	virtual void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects) = 0;
	virtual void SetViewports(unsigned numViewports, gxapi::Viewport* viewports) = 0;

	// set compute root signature stuff
	virtual void SetComputeBinder(Binder* binder) = 0;

	virtual void BindCompute(BindParameter parameter, Texture1D* shaderResource);
	virtual void BindCompute(BindParameter parameter, Texture2D* shaderResource);
	virtual void BindCompute(BindParameter parameter, Texture3D* shaderResource);
	virtual void BindCompute(BindParameter parameter, DisposableConstBuffer* shaderConstant);
	virtual void BindCompute(BindParameter parameter, const void* shaderConstant);
	//virtual void BindCompute(BindParameter parameter, RWTexture1D* rwResource);
	//virtual void BindCompute(BindParameter parameter, RWTexture2D* rwResource);
	//virtual void BindCompute(BindParameter parameter, RWTexture3D* rwResource);

	// set graphics root signature stuff
	virtual void SetGraphicsBinder(Binder* binder) = 0;

	virtual void BindGraphics(BindParameter parameter, Texture1D* shaderResource);
	virtual void BindGraphics(BindParameter parameter, Texture2D* shaderResource);
	virtual void BindGraphics(BindParameter parameter, Texture3D* shaderResource);
	virtual void BindGraphics(BindParameter parameter, DisposableConstBuffer* shaderConstant);
	virtual void BindGraphics(BindParameter parameter, const void* shaderConstant);
	//virtual void BindGraphics(BindParameter parameter, RWTexture1D* rwResource);
	//virtual void BindGraphics(BindParameter parameter, RWTexture2D* rwResource);
	//virtual void BindGraphics(BindParameter parameter, RWTexture3D* rwResource);

	// set pipeline state
	virtual void SetPipelineState(gxapi::IPipelineState* pipelineState) = 0;

private:

};


} // namespace gxeng
} // namespace inl