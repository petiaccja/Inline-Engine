#pragma once

#include "../GraphicsApi_LL/IGraphicsCommandList.hpp"
#include "CommandList.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <wrl.h>
#include <d3d12.h>

namespace inl {
namespace gxapi_dx12 {

using Microsoft::WRL::ComPtr;

class GraphicsCommandList : public CommandList, public gxapi::IGraphicsCommandList {
public:
	GraphicsCommandList(ComPtr<ID3D12GraphicsCommandList>& native);
	GraphicsCommandList(const GraphicsCommandList&) = delete;
	GraphicsCommandList& operator=(const GraphicsCommandList&) = delete;

	ID3D12CommandList* GetNativeGenericList() override;
	ID3D12GraphicsCommandList* GetNative();

	gxapi::eCommandListType GetType() override;

	// Command list state
	void ResetState(gxapi::IPipelineState* newState = nullptr) override;
	void Close() override;
	void Reset(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* newState = nullptr) override;

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


	// Resource copy
	void CopyBuffer(gxapi::IResource* dst, size_t dstOffset, gxapi::IResource* src, size_t srcOffset, size_t numBytes) override;

	void CopyResource(gxapi::IResource* dst, gxapi::IResource* src) override;

	void CopyTexture(gxapi::IResource* dst,
		unsigned dstSubresourceIndex,
		gxapi::IResource* src,
		unsigned srcSubresourceIndex) override;

	void CopyTexture(gxapi::IResource* dst,
		gxapi::TextureCopyDesc dstDesc,
		gxapi::IResource* src,
		gxapi::TextureCopyDesc srcDesc,
		int offx, int offy, int offz,
		gxapi::Cube region) override;

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

	// barriers
	// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions


	// rasterizer state
	void SetScissorRects(unsigned numRects, gxapi::Rectangle* rects) override;
	void SetViewports(unsigned numViewports, gxapi::Viewport* viewports) override;

	// set compute root signature stuff

	// set graphics root signature stuff
	void SetGraphicsRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) override;
	void SetGraphicsRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, uint32_t* value) override;
	void SetGraphicsRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) override;
	void SetGraphicsRootDescriptorTable(unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) override;
	void SetGraphicsRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) override;

	void SetGraphicsRootSignature(gxapi::IRootSignature* rootSignature) override;

	// set pipeline state
	void SetPipelineState(gxapi::IPipelineState* pipelineState) override;



protected:
	ComPtr<ID3D12GraphicsCommandList> m_native;

protected:
	static D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* texture, gxapi::TextureCopyDesc descrition);
	static D3D12_TEXTURE_COPY_LOCATION CreateTextureCopyLocation(gxapi::IResource* texture, unsigned subresourceIndex);
};


} //namespace gxapi_dx12
} //namespace inl
