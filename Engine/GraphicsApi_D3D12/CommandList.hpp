#pragma once

#include "../GraphicsApi_LL/ICommandList.hpp"
#include "CommandList.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>
#include <WinPixEventRuntime/pix3.h>
#include "../GraphicsApi_LL/DisableWin32Macros.h"

#pragma warning(disable: 4250)

namespace inl {
namespace gxapi {

class IDescriptorHeap;

}
}

namespace inl {
namespace gxapi_dx12 {


using Microsoft::WRL::ComPtr;



class BasicCommandList : virtual public gxapi::ICommandList {
public:
	BasicCommandList(ComPtr<ID3D12GraphicsCommandList>& native);

	virtual ~BasicCommandList() = default;
	ID3D12CommandList* GetNativeGenericList();
	ID3D12GraphicsCommandList* GetNative();

	gxapi::eCommandListType GetType() const override;

	void BeginDebuggerEvent(const std::string& name) const override;
	void EndDebuggerEvent() const override;

	void SetName(const char* name) override;
protected:
	ComPtr<ID3D12GraphicsCommandList> m_native;
};



class CopyCommandList : public BasicCommandList, virtual public gxapi::ICopyCommandList {
public:
	// basic
	CopyCommandList(ComPtr<ID3D12GraphicsCommandList>& native);


	// Command list state
	void Close() override;
	void Reset(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* newState = nullptr) override;


	// Resource copy
	void CopyBuffer(gxapi::IResource* dst,
					size_t dstOffset,
					gxapi::IResource* src,
					size_t srcOffset,
					size_t numBytes) override;

	void CopyResource(gxapi::IResource* dst, gxapi::IResource* src) override;

	void CopyTexture(gxapi::IResource* dst,
					 unsigned dstSubresourceIndex,
					 int dstX, int dstY, int dstZ,
					 gxapi::IResource* src,
					 unsigned srcSubresourceIndex,
					 gxapi::Cube srcRegion) override;

	void CopyTexture(gxapi::IResource* dst,
					 gxapi::TextureCopyDesc dstDesc,
					 int dstX, int dstY, int dstZ,
					 gxapi::IResource* src,
					 gxapi::TextureCopyDesc srcDesc,
					 gxapi::Cube srcRegion) override;

	void CopyTexture(gxapi::IResource* dst,
	                 gxapi::TextureCopyDesc dstDesc,
	                 int dstX, int dstY, int dstZ,
	                 gxapi::IResource* src,
	                 gxapi::TextureCopyDesc srcDesc) override;

	// barriers
	// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions
	void ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) override;

protected:
	D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* resource, gxapi::TextureCopyDesc descrition);
	D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* texture, unsigned subresourceIndex);
};



class ComputeCommandList : public CopyCommandList, virtual public gxapi::IComputeCommandList {	
public:
	ComputeCommandList(ComPtr<ID3D12GraphicsCommandList>& native);

	// draw
	void Dispatch(size_t dimx, size_t dimy = 1, size_t dimz = 1) override;

	// set compute root signature stuff
	void SetComputeRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) override;
	void SetComputeRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) override;
	void SetComputeRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) override;
	void SetComputeRootDescriptorTable(unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) override;
	void SetComputeRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) override;
	void SetComputeRootUnorderedResource(unsigned parameterIndex, void* gpuVirtualAddress) override;

	void SetComputeRootSignature(gxapi::IRootSignature* rootSignature) override;

	// set pipeline state
	void SetPipelineState(gxapi::IPipelineState* pipelineState) override;
	void ResetState(gxapi::IPipelineState* initialPipelineState) override;

	// descriptor heaps
	void SetDescriptorHeaps(gxapi::IDescriptorHeap*const * heaps, uint32_t count) override;
};



class GraphicsCommandList : public ComputeCommandList, virtual public gxapi::IGraphicsCommandList {
public:
	GraphicsCommandList(ComPtr<ID3D12GraphicsCommandList>& native);

	// Clear shit
	void ClearDepthStencil(gxapi::DescriptorHandle dsv,
						   float depth,
						   uint8_t stencil,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr,
						   bool clearDepth = true,
						   bool clearStencil = false) override;

	void ClearRenderTarget(gxapi::DescriptorHandle rtv,
						   gxapi::ColorRGBA color,
						   size_t numRects = 0,
						   gxapi::Rectangle* rects = nullptr) override;


	// Draw
	void DrawIndexedInstanced(unsigned numIndices,
							  unsigned startIndex = 0,
							  int vertexOffset = 0,
							  unsigned numInstances = 1,
							  unsigned startInstance = 0) override;

	void DrawInstanced(unsigned numVertices,
					   unsigned startVertex = 0,
					   unsigned numInstances = 1,
					   unsigned startInstance = 0) override;

	void ExecuteBundle(IGraphicsCommandList* bundle) override;

	// input assembler
	void SetIndexBuffer(void* gpuVirtualAddress, size_t sizeInBytes, gxapi::eFormat format) override;

	void SetPrimitiveTopology(gxapi::ePrimitiveTopology topology) override;

	void SetVertexBuffers(unsigned startSlot,
						  unsigned count,
						  void** gpuVirtualAddress,
						  unsigned* sizeInBytes,
						  unsigned* strideInBytes) override;

	// output merger
	void SetRenderTargets(unsigned numRenderTargets,
						  gxapi::DescriptorHandle* renderTargets,
						  gxapi::DescriptorHandle* depthStencil = nullptr) override;
	void SetBlendFactor(float r, float g, float b, float a) override;
	void SetStencilRef(unsigned stencilRef) override;


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects) override;
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports) override;


	// set graphics root signature stuff
	void SetGraphicsRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) override;
	void SetGraphicsRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) override;
	void SetGraphicsRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) override;
	void SetGraphicsRootDescriptorTable(unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) override;
	void SetGraphicsRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) override;

	void SetGraphicsRootSignature(gxapi::IRootSignature* rootSignature) override;
};


#pragma warning(default: 4250)



} //namespace gxapi_dx12
} //namespace inl
