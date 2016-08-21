#pragma once

#include "ComputeCommandList.hpp"


namespace inl {
namespace gxeng {



class GraphicsCommandList : public ComputeCommandList {
public:
	GraphicsCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool);
	GraphicsCommandList(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList(GraphicsCommandList&& rhs);
	GraphicsCommandList& operator=(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList& operator=(GraphicsCommandList&& rhs);

public:
	// Clear shit
	void ClearDepthStencil(Texture2D* resource,
						   float depth,
						   uint8_t stencil,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr,
						   bool clearDepth = true,
						   bool clearStencil = false);

	void ClearRenderTarget(Texture2D* resource,
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
	void SetIndexBuffer(IndexBuffer* resource, bool is32Bit);

	void SetPrimitiveTopology(gxapi::ePrimitiveTopology topology);

	void SetVertexBuffers(unsigned startSlot,
						  unsigned count,
						  VertexBuffer** resources,
						  unsigned* sizeInBytes,
						  unsigned* strideInBytes);

	// output merger
	void SetRenderTargets(unsigned numRenderTargets,
						  Texture2D** renderTargets,
						  Texture2D* depthStencil = nullptr);
	void SetBlendFactor(float r, float g, float b, float a);
	void SetStencilRef(unsigned stencilRef);


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects);
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports);


	// set graphics root signature stuff
	void SetGraphicsBinder(Binder* binder);

	void BindGraphics(BindParameter parameter, Texture1D* shaderResource);
	void BindGraphics(BindParameter parameter, Texture2D* shaderResource);
	void BindGraphics(BindParameter parameter, Texture3D* shaderResource);
	void BindGraphics(BindParameter parameter, DisposableConstBuffer* shaderConstant);
	void BindGraphics(BindParameter parameter, const void* shaderConstant);
	//void BindGraphics(BindParameter parameter, RWTexture1D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture2D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture3D* rwResource);


	// set pipeline state
	void SetPipelineState(gxapi::IPipelineState* pipelineState);

protected:
	virtual Decomposition Decompose() override;
private:
	gxapi::IGraphicsCommandList* m_commandList;
};



} // namespace gxeng
} // namespace inl