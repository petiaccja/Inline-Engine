#pragma once

#include "Common.hpp"

#include "../BaseLibrary/TemplateUtil.hpp"

namespace inl {
namespace gxapi {

class IDescriptorHeap;

class ICommandList {
public:
	virtual ~ICommandList() = default;

	virtual eCommandListType GetType() const = 0;

	virtual void BeginDebuggerEvent(const std::string& name) const = 0;
	virtual void EndDebuggerEvent() const = 0;

	// Debug
	virtual void SetName(const char* name) = 0;
};



class ICopyCommandList : virtual public ICommandList {
public:
	virtual ~ICopyCommandList() = default;

	// Command list state
	virtual void Close() = 0;
	virtual void Reset(ICommandAllocator* allocator, IPipelineState* newState = nullptr) = 0;


	// Resource copy
	virtual void CopyBuffer(IResource* dst, size_t dstOffset, IResource* src, size_t srcOffset, size_t numBytes) = 0;

	virtual void CopyResource(IResource* dst, IResource* src) = 0;

	virtual void CopyTexture(IResource* dst,
							 unsigned dstSubresourceIndex,
							 int dstX, int dstY, int dstZ,
							 IResource* src,
							 unsigned srcSubresourceIndex,
							 Cube srcRegion) = 0;

	virtual void CopyTexture(IResource* dst,
							 TextureCopyDesc dstDesc,
							 int dstX, int dstY, int dstZ,
							 IResource* src,
							 TextureCopyDesc srcDesc,
							 Cube srcRegion) = 0;

	virtual void CopyTexture(gxapi::IResource* dst,
	                         gxapi::TextureCopyDesc dstDesc,
	                         int dstX, int dstY, int dstZ,
	                         gxapi::IResource* src,
	                         gxapi::TextureCopyDesc srcDesc) = 0;

	// barriers
	// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions
	virtual void ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) = 0;

	template <class... Barriers>
	std::enable_if_t<
		templ::all<std::is_base_of<ResourceBarrierTag, std::remove_reference_t<Barriers>>...>::value,
		void
	>
	ResourceBarrier(Barriers&&... barriers) {
		constexpr unsigned int tableSize = sizeof...(Barriers);
		::inl::gxapi::ResourceBarrier table[tableSize];
		PopulateBarrierTable(table, std::forward<Barriers&&>(barriers)...);
		ResourceBarrier(tableSize, (gxapi::ResourceBarrier*)table);
	}

protected:
	template <class Head, class... Barriers>
	void PopulateBarrierTable(::inl::gxapi::ResourceBarrier* target, const Head& head, Barriers&&... tail);
	void PopulateBarrierTable(::inl::gxapi::ResourceBarrier* target) {}
};



class IComputeCommandList : virtual public ICopyCommandList {
public:
	virtual ~IComputeCommandList() = default;

	// draw
	virtual void Dispatch(size_t dimx, size_t dimy = 1, size_t dimz = 1) = 0;

	// set compute root signature stuff
	virtual void SetComputeRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) = 0;
	virtual void SetComputeRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) = 0;
	virtual void SetComputeRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) = 0;
	virtual void SetComputeRootDescriptorTable(unsigned parameterIndex, DescriptorHandle baseHandle) = 0;
	virtual void SetComputeRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) = 0;
	virtual void SetComputeRootUnorderedResource(unsigned parameterIndex, void* gpuVirtualAddress) = 0;

	virtual void SetComputeRootSignature(IRootSignature* rootSignature) = 0;

	// set pipeline state
	virtual void SetPipelineState(IPipelineState* pipelineState) = 0;
	virtual void ResetState(IPipelineState* initialPipelineState) = 0;

	// descriptor heaps
	virtual void SetDescriptorHeaps(IDescriptorHeap*const * heaps, uint32_t count) = 0;
};



class IGraphicsCommandList : virtual public IComputeCommandList {
public:
	virtual ~IGraphicsCommandList() = default;


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


	// rasterizer state
	virtual void SetScissorRects(unsigned numRects, Rectangle* rects) = 0;
	virtual void SetViewports(unsigned numViewports, Viewport* viewports) = 0;


	// set graphics root signature stuff
	virtual void SetGraphicsRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) = 0;
	virtual void SetGraphicsRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) = 0;
	virtual void SetGraphicsRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) = 0;
	virtual void SetGraphicsRootDescriptorTable(unsigned parameterIndex, DescriptorHandle baseHandle) = 0;
	virtual void SetGraphicsRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) = 0;

	virtual void SetGraphicsRootSignature(IRootSignature* rootSignature) = 0;
};


template <class Head, class... Barriers>
void ICopyCommandList::PopulateBarrierTable(::inl::gxapi::ResourceBarrier* target, const Head& head, Barriers&&... tail) {
	*target = ::inl::gxapi::ResourceBarrier(head);

	PopulateBarrierTable(target + 1, std::forward<Barriers&&>(tail)...);
}



} // namespace gxapi
} // namespace inl
