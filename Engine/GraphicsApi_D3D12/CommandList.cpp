#include "CommandList.hpp"

#include "PipelineState.hpp"
#include "NativeCast.hpp"
#include "ExceptionExpansions.hpp"

#include "../GraphicsApi_LL/Common.hpp"

#include <vector>
#include <cassert>

#define _CRT_SECURE_NO_WARNINGS

namespace inl {
namespace gxapi_dx12 {



//------------------------------------------------------------------------------
// Basic command list
//------------------------------------------------------------------------------


BasicCommandList::BasicCommandList(ComPtr<ID3D12GraphicsCommandList>& native)
	: m_native(native)
{}


ID3D12CommandList* BasicCommandList::GetNativeGenericList() {
	return m_native.Get();
}

ID3D12GraphicsCommandList* BasicCommandList::GetNative() {
	return m_native.Get();
}



gxapi::eCommandListType BasicCommandList::GetType() const {
	return native_cast(m_native->GetType());
}


void BasicCommandList::BeginDebuggerEvent(const std::string& name) const {
	PIXBeginEvent(m_native.Get(), PIX_COLOR_DEFAULT, name.c_str());
}

void BasicCommandList::EndDebuggerEvent() const {
	PIXEndEvent(m_native.Get());
}

void BasicCommandList::SetName(const char* name) {
	size_t count = strlen(name);
	std::unique_ptr<wchar_t[]> dest = std::make_unique<wchar_t[]>(count + 1);
	mbstowcs(dest.get(), name, count);
	m_native->SetName(dest.get());
}



//------------------------------------------------------------------------------
// Copy command list
//------------------------------------------------------------------------------


// basic
CopyCommandList::CopyCommandList(ComPtr<ID3D12GraphicsCommandList>& native)
	: BasicCommandList(native)
{

	assert(native->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT ||
		   native->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE ||
		   native->GetType() == D3D12_COMMAND_LIST_TYPE_COPY);
}


// Command list state
void CopyCommandList::Close() {
	ThrowIfFailed(m_native->Close());
}

void CopyCommandList::Reset(gxapi::ICommandAllocator* allocator, gxapi::IPipelineState* newState) {
	ThrowIfFailed(m_native->Reset(native_cast(allocator), native_cast(newState)));
}


// Resource copy
void CopyCommandList::CopyBuffer(
	gxapi::IResource* dst,
	size_t dstOffset,
	gxapi::IResource* src,
	size_t srcOffset,
	size_t numBytes
) {
	m_native->CopyBufferRegion(native_cast(dst), dstOffset, native_cast(src), srcOffset, numBytes);
}

void CopyCommandList::CopyResource(gxapi::IResource* dst, gxapi::IResource* src) {
	m_native->CopyResource(native_cast(dst), native_cast(src));
}

void CopyCommandList::CopyTexture(
	gxapi::IResource* dst,
	unsigned dstSubresourceIndex,
	int dstX, int dstY, int dstZ,
	gxapi::IResource* src,
	unsigned srcSubresourceIndex,
	gxapi::Cube srcRegion
) {
	auto nativeDst = CreateTextureCopyLocation(dst, dstSubresourceIndex);
	auto nativeSrc = CreateTextureCopyLocation(src, srcSubresourceIndex);

	m_native->CopyTextureRegion(&nativeDst, 0, 0, 0, &nativeSrc, nullptr);
}

void CopyCommandList::CopyTexture(
	gxapi::IResource* dst,
	gxapi::TextureCopyDesc dstDesc,
	int dstX, int dstY, int dstZ,
	gxapi::IResource* src,
	gxapi::TextureCopyDesc srcDesc,
	gxapi::Cube srcRegion
) {
	auto nativeDst = CreateTextureCopyLocation(dst, dstDesc);
	auto nativeSrc = CreateTextureCopyLocation(src, srcDesc);

	D3D12_BOX srcBox = native_cast(srcRegion);

	m_native->CopyTextureRegion(&nativeDst, dstX, dstY, dstZ, &nativeSrc, &srcBox);
}


void CopyCommandList::CopyTexture(
	gxapi::IResource* dst,
	gxapi::TextureCopyDesc dstDesc,
	int dstX, int dstY, int dstZ,
	gxapi::IResource* src,
	gxapi::TextureCopyDesc srcDesc
) {
	auto nativeDst = CreateTextureCopyLocation(dst, dstDesc);
	auto nativeSrc = CreateTextureCopyLocation(src, srcDesc);

	m_native->CopyTextureRegion(&nativeDst, dstX, dstY, dstZ, &nativeSrc, nullptr);
}


// barriers
// TODO: transition, aliasing and bullshit barriers, i would put them into separate functions
void CopyCommandList::ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) {
	std::vector<D3D12_RESOURCE_BARRIER> nativeBarriers(numBarriers);
	for (unsigned i = 0; i < numBarriers; i++) {
		nativeBarriers[i] = native_cast(barriers[i]);
	}
	m_native->ResourceBarrier(numBarriers, nativeBarriers.data());
}


// helpers
D3D12_TEXTURE_COPY_LOCATION CopyCommandList::CreateTextureCopyLocation(gxapi::IResource* resource, gxapi::TextureCopyDesc description) {

	D3D12_TEXTURE_COPY_LOCATION result;
	{
		result.pResource = native_cast(resource);

		D3D12_RESOURCE_DESC resourceDesc = native_cast(resource)->GetDesc();
		const bool isTexture =
			resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D
			|| resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D
			|| resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D;

		if (isTexture) {
			result.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			result.SubresourceIndex = description.subresourceIndex;
		}
		else {
			assert(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER);
			result.Type = D3D12_TEXTURE_COPY_TYPE::D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint;

			D3D12_SUBRESOURCE_FOOTPRINT footprint;
			{
				footprint.Depth = description.depth;
				footprint.Format = native_cast(description.format);
				footprint.Height = description.height;
				footprint.Width = (UINT)description.width; // narrowing conversion!
				size_t rowSize = size_t(GetFormatSizeInBytes(description.format)*description.width);
				size_t alignement = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
				footprint.RowPitch = static_cast<UINT>(rowSize + (alignement - rowSize % alignement) % alignement);
			}
			placedFootprint.Footprint = footprint;
			placedFootprint.Offset = description.byteOffset;

			result.PlacedFootprint = placedFootprint;
		}
	}

	return result;
}


D3D12_TEXTURE_COPY_LOCATION CopyCommandList::CreateTextureCopyLocation(gxapi::IResource* texture, unsigned subresourceIndex)
{
	D3D12_TEXTURE_COPY_LOCATION result;
	result.pResource = native_cast(texture);
	result.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	result.SubresourceIndex = subresourceIndex;

	return result;
}


//------------------------------------------------------------------------------
// Compute command list
//------------------------------------------------------------------------------


// basic
ComputeCommandList::ComputeCommandList(ComPtr<ID3D12GraphicsCommandList>& native)
	: CopyCommandList(native)
{
	assert(native->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT ||
		   native->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE);
}


// draw
void ComputeCommandList::Dispatch(size_t numThreadGroupsX, size_t numThreadGroupsY, size_t numThreadGroupsZ) {
	m_native->Dispatch((UINT)numThreadGroupsX, (UINT)numThreadGroupsY, (UINT)numThreadGroupsZ);
}


// set graphics root signature stuff
void ComputeCommandList::SetComputeRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) {
	m_native->SetComputeRoot32BitConstant(parameterIndex, value, destOffset);
}


void ComputeCommandList::SetComputeRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) {
	m_native->SetComputeRoot32BitConstants(parameterIndex, numValues, value, destOffset);
}


void ComputeCommandList::SetComputeRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) {
	m_native->SetComputeRootConstantBufferView(parameterIndex, native_cast_ptr(gpuVirtualAddress));
}


void ComputeCommandList::SetComputeRootDescriptorTable(unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) {
	D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor;
	baseDescriptor.ptr = native_cast_ptr(baseHandle.gpuAddress);

	m_native->SetComputeRootDescriptorTable(parameterIndex, baseDescriptor);
}


void ComputeCommandList::SetComputeRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) {
	m_native->SetComputeRootShaderResourceView(parameterIndex, native_cast_ptr(gpuVirtualAddress));
}


void ComputeCommandList::SetComputeRootUnorderedResource(unsigned parameterIndex, void* gpuVirtualAddress) {
	m_native->SetComputeRootUnorderedAccessView(parameterIndex, native_cast_ptr(gpuVirtualAddress));
}


void ComputeCommandList::SetComputeRootSignature(gxapi::IRootSignature* rootSignature) {
	m_native->SetComputeRootSignature(native_cast(rootSignature));
}


void ComputeCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_native->ClearState(native_cast(newState));
}


// set pipeline state
void ComputeCommandList::SetPipelineState(gxapi::IPipelineState * pipelineState) {
	m_native->SetPipelineState(native_cast(pipelineState));
}


// set descriptor heaps
void ComputeCommandList::SetDescriptorHeaps(gxapi::IDescriptorHeap*const * heaps, uint32_t count) {

	std::vector<ID3D12DescriptorHeap*> nativeHeaps;
	nativeHeaps.reserve(count);

	for (unsigned i = 0; i < count; i++) {
		nativeHeaps.push_back(native_cast(heaps[i]));
	}

	m_native->SetDescriptorHeaps(count, nativeHeaps.data());
}


//------------------------------------------------------------------------------
// Graphics command list
//------------------------------------------------------------------------------


// basic
GraphicsCommandList::GraphicsCommandList(ComPtr<ID3D12GraphicsCommandList>& native)
	: ComputeCommandList(native)
{
	assert(native->GetType() == D3D12_COMMAND_LIST_TYPE_DIRECT);
}


// Clear shit
void GraphicsCommandList::ClearDepthStencil(
	gxapi::DescriptorHandle dsv,
	float depth,
	uint8_t stencil,
	size_t numRects,
	gxapi::Rectangle* rects,
	bool clearDepth, bool clearStencil) {

	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
	depthStencilView.ptr = native_cast_ptr(dsv.cpuAddress);

	D3D12_CLEAR_FLAGS flags;
	flags = static_cast<D3D12_CLEAR_FLAGS>(clearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0);
	flags = static_cast<D3D12_CLEAR_FLAGS>(flags | (clearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0));

	std::vector<D3D12_RECT> castedRects;
	castedRects.reserve(numRects);
	for (size_t i = 0; i < numRects; i++) {
		castedRects.push_back(native_cast(rects[i]));
	}

	D3D12_RECT* pRects = (castedRects.size() == 0) ? nullptr : castedRects.data();

	m_native->ClearDepthStencilView(depthStencilView, flags, depth, stencil, (UINT)castedRects.size(), pRects);
}


void GraphicsCommandList::ClearRenderTarget(gxapi::DescriptorHandle rtv, gxapi::ColorRGBA color, size_t numRects, gxapi::Rectangle* rects) {
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView;
	renderTargetView.ptr = native_cast_ptr(rtv.cpuAddress);

	float nativeColor[4] = { color.r, color.g, color.b, color.a };

	std::vector<D3D12_RECT> castedRects;
	castedRects.reserve(numRects);
	for (size_t i = 0; i < numRects; i++) {
		castedRects.push_back(native_cast(rects[i]));
	}

	D3D12_RECT* pRects = (castedRects.size() == 0) ? nullptr : castedRects.data();

	m_native->ClearRenderTargetView(renderTargetView, nativeColor, (UINT)castedRects.size(), pRects);
}


// Draw
void GraphicsCommandList::DrawIndexedInstanced(unsigned numIndices, unsigned startIndex, int vertexOffset, unsigned numInstances, unsigned startInstance) {
	m_native->DrawIndexedInstanced(numIndices, numInstances, startIndex, vertexOffset, startInstance);
}


void GraphicsCommandList::DrawInstanced(unsigned numVertices, unsigned startVertex, unsigned numInstances, unsigned startInstance) {
	m_native->DrawInstanced(numVertices, numInstances, startVertex, startInstance);
}


void GraphicsCommandList::ExecuteBundle(IGraphicsCommandList* bundle) {
	m_native->ExecuteBundle(native_cast(bundle));
}


// Input assembler
void GraphicsCommandList::SetIndexBuffer(void * gpuVirtualAddress, size_t sizeInBytes, gxapi::eFormat format) {
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = native_cast_ptr(gpuVirtualAddress);
	ibv.Format = native_cast(format);
	ibv.SizeInBytes = static_cast<UINT>(sizeInBytes);

	m_native->IASetIndexBuffer(&ibv);
}


void GraphicsCommandList::SetPrimitiveTopology(gxapi::ePrimitiveTopology topology) {
	m_native->IASetPrimitiveTopology(native_cast(topology));
}


void GraphicsCommandList::SetVertexBuffers(unsigned startSlot, unsigned count, void** gpuVirtualAddress, unsigned* sizeInBytes, unsigned* strideInBytes) {
	std::vector<D3D12_VERTEX_BUFFER_VIEW> views;
	views.reserve(count);

	for (unsigned i = 0; i < count; i++) {
		D3D12_VERTEX_BUFFER_VIEW tmpView;
		tmpView.BufferLocation = native_cast_ptr(gpuVirtualAddress[i]);
		tmpView.SizeInBytes = sizeInBytes[i];
		tmpView.StrideInBytes = strideInBytes[i];

		views.push_back(std::move(tmpView));
	}

	m_native->IASetVertexBuffers(startSlot, (UINT)views.size(), views.data());
}


// Output merger
void GraphicsCommandList::SetRenderTargets(unsigned numRenderTargets, gxapi::DescriptorHandle* renderTargets, gxapi::DescriptorHandle* depthStencil) {
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
	renderTargetDescriptors.reserve(numRenderTargets);
	for (unsigned i = 0; i < numRenderTargets; i++) {
		D3D12_CPU_DESCRIPTOR_HANDLE tmpDescriptor;
		tmpDescriptor.ptr = native_cast_ptr(renderTargets[i].cpuAddress);
		renderTargetDescriptors.push_back(tmpDescriptor);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor;
	if (depthStencil != nullptr) {
		depthStencilDescriptor.ptr = native_cast_ptr(depthStencil->cpuAddress);
		pDepthStencilDescriptor = &depthStencilDescriptor;
	}

	m_native->OMSetRenderTargets((UINT)renderTargetDescriptors.size(), renderTargetDescriptors.data(), false, pDepthStencilDescriptor);
}


void GraphicsCommandList::SetBlendFactor(float r, float g, float b, float a) {
	float native[4] = { r, g, b, a };
	m_native->OMSetBlendFactor(native);
}


void GraphicsCommandList::SetStencilRef(unsigned stencilRef) {
	m_native->OMSetStencilRef(stencilRef);
}



// Rasterizer
void GraphicsCommandList::SetScissorRects(unsigned numRects, gxapi::Rectangle* rects) {
	std::vector<D3D12_RECT> nativeRects;
	nativeRects.reserve(numRects);

	for (unsigned i = 0; i < numRects; i++) {
		nativeRects.push_back(native_cast(rects[i]));
	}

	m_native->RSSetScissorRects((UINT)nativeRects.size(), nativeRects.data());
}


void GraphicsCommandList::SetViewports(unsigned numViewports, gxapi::Viewport* viewports) {
	std::vector<D3D12_VIEWPORT> nativeViewports;
	nativeViewports.reserve(numViewports);

	for (unsigned i = 0; i < numViewports; i++) {
		nativeViewports.push_back(native_cast(viewports[i]));
	}

	m_native->RSSetViewports((UINT)nativeViewports.size(), nativeViewports.data());
}


// set graphics root signature stuff
void GraphicsCommandList::SetGraphicsRootConstant(unsigned parameterIndex, unsigned destOffset, uint32_t value) {
	m_native->SetGraphicsRoot32BitConstant(parameterIndex, value, destOffset);
}


void GraphicsCommandList::SetGraphicsRootConstants(unsigned parameterIndex, unsigned destOffset, unsigned numValues, const uint32_t* value) {
	m_native->SetGraphicsRoot32BitConstants(parameterIndex, numValues, value, destOffset);
}


void GraphicsCommandList::SetGraphicsRootConstantBuffer(unsigned parameterIndex, void* gpuVirtualAddress) {
	m_native->SetGraphicsRootConstantBufferView(parameterIndex, native_cast_ptr(gpuVirtualAddress));
}


void GraphicsCommandList::SetGraphicsRootDescriptorTable(unsigned parameterIndex, gxapi::DescriptorHandle baseHandle) {
	D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor;
	baseDescriptor.ptr = native_cast_ptr(baseHandle.gpuAddress);

	m_native->SetGraphicsRootDescriptorTable(parameterIndex, baseDescriptor);
}


void GraphicsCommandList::SetGraphicsRootShaderResource(unsigned parameterIndex, void* gpuVirtualAddress) {
	m_native->SetGraphicsRootShaderResourceView(parameterIndex, native_cast_ptr(gpuVirtualAddress));
}


void GraphicsCommandList::SetGraphicsRootSignature(gxapi::IRootSignature* rootSignature) {
	m_native->SetGraphicsRootSignature(native_cast(rootSignature));
}



} // namespace gxapi_dx12
} // namespace inl
