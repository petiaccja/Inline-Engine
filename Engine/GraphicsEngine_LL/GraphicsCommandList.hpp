#pragma once

#include "ComputeCommandList.hpp"
#include "ResourceView.hpp"
#include "PipelineEventListener.hpp"

namespace inl {
namespace gxeng {



class GraphicsCommandList : public ComputeCommandList {
	struct DescriptorTableState {
		DescriptorTableState(ScratchSpaceRef&& reference, bool persistent, int slot)
			: reference(std::move(reference)), persistent(persistent), slot(slot)
		{}

		ScratchSpaceRef reference;
		bool persistent;
		int slot;

		std::vector<gxapi::DescriptorHandle> boundDescriptors;
	};
public:
	GraphicsCommandList(
		gxapi::IGraphicsApi* gxApi,
		CommandAllocatorPool& commandAllocatorPool,
		ScratchSpacePool& scratchSpacePool);
	GraphicsCommandList(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList(GraphicsCommandList&& rhs);
	GraphicsCommandList& operator=(const GraphicsCommandList& rhs) = delete;
	GraphicsCommandList& operator=(GraphicsCommandList&& rhs);

public:
	// Clear shit
	void ClearDepthStencil(DepthStencilView& resource,
						   float depth,
						   uint8_t stencil,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr,
						   bool clearDepth = true,
						   bool clearStencil = false);

	void ClearRenderTarget(RenderTargetView& resource,
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
						  RenderTargetView** renderTargets,
						  DepthStencilView* depthStencil = nullptr);
	void SetBlendFactor(float r, float g, float b, float a);
	void SetStencilRef(unsigned stencilRef);


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects);
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports);


	// set graphics root signature stuff
	void SetGraphicsBinder(Binder* binder);

	void BindGraphics(BindParameter parameter, const Texture1DSRV& shaderResource);
	void BindGraphics(BindParameter parameter, const Texture2DSRV& shaderResource);
	void BindGraphics(BindParameter parameter, const Texture3DSRV& shaderResource);


#define TO_STRING(X) #X
#define TO_STR_ANY(X) TO_STRING(X)
#pragma message(__FILE__ "(" TO_STR_ANY(__LINE__) "): warning: PETI ezt nem írtam át, rádbízom")
	void BindGraphics(BindParameter parameter, ConstBuffer* shaderConstant);
#undef TO_STRING
#undef TO_STR_ANY


	void BindGraphics(BindParameter parameter, const void* shaderConstant, int size, int offset);
	//void BindGraphics(BindParameter parameter, RWTexture1D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture2D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture3D* rwResource);


	void DEBUG_SetGraphicsRootSignature(gxapi::IRootSignature* rootsig) {
		m_commandList->SetGraphicsRootSignature(rootsig);
	}
protected:
	virtual Decomposition Decompose() override;
private:
	void BindGraphicsTexture(BindParameter parameter, gxapi::DescriptorHandle handle);
	void WriteScratchSpace(gxapi::DescriptorHandle, int slot, int index);
	void DuplicateDescriptors();
	void InitializeDescTableStates();
private:
	gxapi::IGraphicsApi *m_graphicsApi;
	gxapi::IGraphicsCommandList* m_commandList;
	Binder* m_binder = nullptr;
	std::vector<DescriptorTableState> m_tableStates;
};



} // namespace gxeng
} // namespace inl