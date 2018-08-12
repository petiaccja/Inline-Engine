#pragma once

#include "ComputeCommandList.hpp"
#include "ResourceView.hpp"
#include "PipelineEventListener.hpp"
#include "StackDescHeap.hpp"
#include "BindingManager.hpp"
#include "Cubemap.hpp"

namespace inl {
namespace gxeng {



class GraphicsCommandList : public ComputeCommandList {
public:
	GraphicsCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandListPool& commandListPool,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool,
		MemoryManager& memoryManager,
		VolatileViewHeap& volatileCbvHeap);
	GraphicsCommandList(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList(GraphicsCommandList&& rhs);
	GraphicsCommandList& operator=(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList& operator=(GraphicsCommandList&& rhs);

public:
	// Clear shit
	void ClearDepthStencil(const DepthStencilView2D& resource,
						   float depth,
						   uint8_t stencil,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr,
						   bool clearDepth = true,
						   bool clearStencil = false);

	void ClearRenderTarget(const RenderTargetView2D& resource,
						   gxapi::ColorRGBA color,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr);


	// Draw
	void DrawIndexedInstanced(unsigned numIndices,
							  unsigned startIndex = 0,
							  int vertexOffset = 0,
							  unsigned numInstances = 1,
							  unsigned startInstance = 0);

	void DrawInstanced(unsigned numVertices,
					   unsigned startVertex = 0,
					   unsigned numInstances = 1,
					   unsigned startInstance = 0);

	//!!! void ExecuteBundle(IGraphicsCommandList* bundle);

	// input assembler
	void SetIndexBuffer(const IndexBuffer* resource, bool is32Bit);

	void SetPrimitiveTopology(gxapi::ePrimitiveTopology topology);

	void SetVertexBuffers(unsigned startSlot,
						  unsigned count,
						  const VertexBuffer* const * resources,
						  unsigned* sizeInBytes,
						  unsigned* strideInBytes);

	// output merger
	void SetRenderTargets(unsigned numRenderTargets,
						  const RenderTargetView2D* const * renderTargets,
						  const DepthStencilView2D* depthStencil = nullptr);
	void SetBlendFactor(float r, float g, float b, float a);
	void SetStencilRef(unsigned stencilRef);


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects);
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports);


	// set graphics root signature stuff
	void SetGraphicsBinder(const Binder* binder);

	void BindGraphics(BindParameter parameter, const TextureView1D& shaderResource);
	void BindGraphics(BindParameter parameter, const TextureView2D& shaderResource);
	void BindGraphics(BindParameter parameter, const TextureView3D& shaderResource);
	void BindGraphics(BindParameter parameter, const TextureViewCube& shaderResource);
	void BindGraphics(BindParameter parameter, const ConstBufferView& shaderConstant);
	void BindGraphics(BindParameter parameter, const void* shaderConstant, int size/*, int offset*/);
	void BindGraphics(BindParameter parameter, const RWTextureView1D& rwResource);
	void BindGraphics(BindParameter parameter, const RWTextureView2D& rwResource);
	void BindGraphics(BindParameter parameter, const RWTextureView3D& rwResource);
	void BindGraphics(BindParameter parameter, const RWBufferView& rwResource);
protected:
	virtual Decomposition Decompose() override;
	virtual void NewScratchSpace(size_t hint) override;
private:
	gxapi::IGraphicsCommandList* m_commandList;

	// scratch space managment
	BindingManager<gxapi::eCommandListType::GRAPHICS> m_graphicsBindingManager;
};



} // namespace gxeng
} // namespace inl