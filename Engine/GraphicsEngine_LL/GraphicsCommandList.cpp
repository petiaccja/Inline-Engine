#include "GraphicsCommandList.hpp"

namespace inl {
namespace gxeng {

//------------------------------------------------------------------------------
// Basic stuff
//------------------------------------------------------------------------------

GraphicsCommandList::GraphicsCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool)
	: ComputeCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::GRAPHICS)
{
	m_commandList = dynamic_cast<gxapi::IGraphicsCommandList*>(GetCommandList());
}


GraphicsCommandList::GraphicsCommandList(GraphicsCommandList&& rhs)
	: ComputeCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList)
{
	rhs.m_commandList = nullptr;
}


GraphicsCommandList& GraphicsCommandList::operator=(GraphicsCommandList&& rhs) {
	ComputeCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}



BasicCommandList::Decomposition GraphicsCommandList::Decompose() {
	m_commandList = nullptr;

	return ComputeCommandList::Decompose();
}


//------------------------------------------------------------------------------
// Clear buffers
//------------------------------------------------------------------------------
void GraphicsCommandList::ClearDepthStencil(DepthStencilView& resource,
											float depth,
											uint8_t stencil,
											size_t numRects,
											gxapi::Rectangle* rects,
											bool clearDepth,
											bool clearStencil)
{
	m_commandList->ClearDepthStencil(resource.GetHandle(), depth, stencil, numRects, rects, clearDepth, clearStencil);
}

void GraphicsCommandList::ClearRenderTarget(RenderTargetView& resource,
											gxapi::ColorRGBA color,
											size_t numRects,
											gxapi::Rectangle* rects)
{
	m_commandList->ClearRenderTarget(resource.GetHandle(), color, numRects, rects);
}


//------------------------------------------------------------------------------
// Draw
//------------------------------------------------------------------------------

void GraphicsCommandList::DrawIndexedInstanced(unsigned numIndices,
											   unsigned startIndex,
											   int vertexOffset,
											   unsigned numInstances,
											   unsigned startInstance)
{
	m_commandList->DrawIndexedInstanced(numIndices, startIndex, vertexOffset, numInstances, startInstance);
}

void GraphicsCommandList::DrawInstanced(unsigned numVertices,
										unsigned startVertex,
										unsigned numInstances,
										unsigned startInstance)
{
	m_commandList->DrawInstanced(numVertices, startVertex, numInstances, startInstance);
}


//------------------------------------------------------------------------------
// Input assembler
//------------------------------------------------------------------------------

void GraphicsCommandList::SetIndexBuffer(const IndexBuffer* resource, bool is32Bit) {
	m_commandList->SetIndexBuffer(resource->GetVirtualAddress(),
								  resource->GetSize(),
								  is32Bit ? gxapi::eFormat::R32_UINT : gxapi::eFormat::R16_UINT);
}


void GraphicsCommandList::SetPrimitiveTopology(gxapi::ePrimitiveTopology topology) {
	m_commandList->SetPrimitiveTopology(topology);
}


void GraphicsCommandList::SetVertexBuffers(unsigned startSlot,
										   unsigned count,
										   const VertexBuffer* const * resources,
										   unsigned* sizeInBytes,
										   unsigned* strideInBytes)
{
	auto virtualAddresses = std::make_unique<void*[]>(count);

	for (unsigned i = 0; i < count; ++i) {
		virtualAddresses[i] = resources[i]->GetVirtualAddress();
	}

	m_commandList->SetVertexBuffers(startSlot,
									count,
									virtualAddresses.get(),
									sizeInBytes,
									strideInBytes);
}


//------------------------------------------------------------------------------
// Output merger
//------------------------------------------------------------------------------

void GraphicsCommandList::SetRenderTargets(unsigned numRenderTargets,
                                           RenderTargetView** renderTargets,
                                           DepthStencilView* depthStencil)
{
	auto renderTargetHandles = std::make_unique<gxapi::DescriptorHandle[]>(numRenderTargets);
	for (unsigned i = 0; i < numRenderTargets; ++i) {
		renderTargetHandles[i] = renderTargets[i]->GetHandle();
	}

	if (depthStencil) {
		gxapi::DescriptorHandle dsvHandle = depthStencil->GetHandle();
		m_commandList->SetRenderTargets(numRenderTargets,
										renderTargetHandles.get(),
										&dsvHandle);
	}
	else {
		m_commandList->SetRenderTargets(numRenderTargets,
										renderTargetHandles.get(),
										nullptr);
	}
}


void GraphicsCommandList::SetBlendFactor(float r, float g, float b, float a) {
	m_commandList->SetBlendFactor(r, g, b, a);
}


void GraphicsCommandList::SetStencilRef(unsigned stencilRef) {
	m_commandList->SetStencilRef(stencilRef);
}


//------------------------------------------------------------------------------
// Rasterizer state
//------------------------------------------------------------------------------

void GraphicsCommandList::SetScissorRects(unsigned numRects, gxapi::Rectangle* rects) {
	m_commandList->SetScissorRects(numRects, rects);
}


void GraphicsCommandList::SetViewports(unsigned numViewports, gxapi::Viewport* viewports) {
	m_commandList->SetViewports(numViewports, viewports);
}


//------------------------------------------------------------------------------
// Set graphics root signature stuff
//------------------------------------------------------------------------------

void GraphicsCommandList::SetGraphicsBinder(Binder* binder) {
	assert(binder != nullptr);

	m_binder = binder;
	m_commandList->SetGraphicsRootSignature(m_binder->GetRootSignature());
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, Texture1D* shaderResource) {
	throw std::runtime_error("not implemented");
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, Texture2D* shaderResource) {
	throw std::runtime_error("not implemented");
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, Texture3D* shaderResource) {
	throw std::runtime_error("not implemented");
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, ConstBuffer* shaderConstant) {
	throw std::runtime_error("not implemented");
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const void* shaderConstant, int size, int offset) {
	if (size % 4 != 0) {
		throw std::invalid_argument("Size must be a multiple of 4.");
	}

	int slot;
	int tableIndex;
	const gxapi::RootSignatureDesc desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex); // may throw out of range
	if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::CONSTANT) {
		assert(desc.rootParameters[slot].As<gxapi::RootParameterDesc::CONSTANT>().numConstants >= (size + offset) / 4);
		m_commandList->SetGraphicsRootConstants(slot, offset, size / 4, reinterpret_cast<const uint32_t*>(shaderConstant));
	}
	else {
		throw std::invalid_argument("Parameter is not an inline constant.");
	}
}


} // namespace gxeng
} // namespace inl