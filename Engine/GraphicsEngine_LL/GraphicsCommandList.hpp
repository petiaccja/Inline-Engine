#pragma once

#include "ComputeCommandList.hpp"
#include "ResourceView.hpp"
#include "PipelineEventListener.hpp"
#include "StackDescHeap.hpp"

namespace inl {
namespace gxeng {



class GraphicsCommandList : public ComputeCommandList {
	struct DescriptorTableState {
		DescriptorTableState() : slot(0), committed(false) {}
		DescriptorTableState(DescriptorArrayRef&& reference, int slot)
			: reference(std::move(reference)), slot(slot), committed(false)
		{}

		DescriptorArrayRef reference; // current place in scratch space
		int slot; // which root signature slot it belongs to
		bool committed; // true if modifying descriptor in sratch space would break previous draw calls
		std::vector<gxapi::DescriptorHandle> bindings; // currently bound descriptor handle, staging heap sources
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
	void ClearDepthStencil(DepthStencilView2D& resource,
						   float depth,
						   uint8_t stencil,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr,
						   bool clearDepth = true,
						   bool clearStencil = false);

	void ClearRenderTarget(RenderTargetView2D& resource,
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
						  RenderTargetView2D** renderTargets,
						  DepthStencilView2D* depthStencil = nullptr);
	void SetBlendFactor(float r, float g, float b, float a);
	void SetStencilRef(unsigned stencilRef);


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects);
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports);


	// set graphics root signature stuff
	void SetGraphicsBinder(Binder* binder);

	void BindGraphics(BindParameter parameter, const TextureView1D& shaderResource);
	void BindGraphics(BindParameter parameter, const TextureView2D& shaderResource);
	void BindGraphics(BindParameter parameter, const TextureView3D& shaderResource);
	void BindGraphics(BindParameter parameter, const ConstBufferView& shaderConstant);
	void BindGraphics(BindParameter parameter, const void* shaderConstant, int size, int offset);
	//void BindGraphics(BindParameter parameter, RWTexture1D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture2D* rwResource);
	//void BindGraphics(BindParameter parameter, RWTexture3D* rwResource);
protected:
	virtual Decomposition Decompose() override;
private:
	void BindGraphicsTexture(BindParameter parameter, gxapi::DescriptorHandle handle);

	// scratch space managment

	/// <summary> Updates a binding which is managed on the scratch space. </summary>
	void UpdateRootTable(gxapi::DescriptorHandle, int rootSignatureSlot, int indexInTable);
	/// <summary> Updates a binding, handles case where scratch space gets full.
	void UpdateRootTableSafe(gxapi::DescriptorHandle, int rootSignatureSlot, int indexInTable);

	/// <summary> Copies a whole scratch space table to a fresh range in scratch space. </summary>
	void DuplicateRootTable(DescriptorTableState& table);

	/// <summary> Get reference to root table state identified by it's root signature slot. </summary>
	DescriptorTableState&  FindRootTable(int rootSignatureSlot);

	/// <summary> Calculates root table states based on the currently bound Binder. </summary>
	void InitRootTables();

	/// <summary> Marks all root tables committed. Call this after each drawcall. </summary>
	void CommitRootTables();

	/// <summary> Copies ALL scratch space tables to a fresh range. Used after a new scratch space is bound. </summary>
	void RenewRootTables();
private:
	gxapi::IGraphicsApi *m_graphicsApi;
	gxapi::IGraphicsCommandList* m_commandList;
	Binder* m_binder = nullptr;

	// scratch space managment
	std::vector<DescriptorTableState> m_rootTableStates;
};



} // namespace gxeng
} // namespace inl