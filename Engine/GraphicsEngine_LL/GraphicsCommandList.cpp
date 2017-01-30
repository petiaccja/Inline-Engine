#include "GraphicsCommandList.hpp"

namespace inl {
namespace gxeng {

//------------------------------------------------------------------------------
// Basic stuff
//------------------------------------------------------------------------------

GraphicsCommandList::GraphicsCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool
) :
	ComputeCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::GRAPHICS)
{
	m_commandList = dynamic_cast<gxapi::IGraphicsCommandList*>(GetCommandList());
	m_graphicsBindingManager = BindingManager<gxapi::eCommandListType::GRAPHICS>(m_graphicsApi, m_commandList);
	m_graphicsBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
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
void GraphicsCommandList::ClearDepthStencil(DepthStencilView2D& resource,
	float depth,
	uint8_t stencil,
	size_t numRects,
	gxapi::Rectangle* rects,
	bool clearDepth,
	bool clearStencil)
{
	m_commandList->ClearDepthStencil(resource.GetHandle(), depth, stencil, numRects, rects, clearDepth, clearStencil);
}

void GraphicsCommandList::ClearRenderTarget(RenderTargetView2D& resource,
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
	m_graphicsBindingManager.CommitDrawCall();
}

void GraphicsCommandList::DrawInstanced(unsigned numVertices,
	unsigned startVertex,
	unsigned numInstances,
	unsigned startInstance)
{
	m_commandList->DrawInstanced(numVertices, startVertex, numInstances, startInstance);
	m_graphicsBindingManager.CommitDrawCall();
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
	RenderTargetView2D** renderTargets,
	DepthStencilView2D* depthStencil)
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
	m_graphicsBindingManager.SetBinder(binder);
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView1D& shaderResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView2D& shaderResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView3D& shaderResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const ConstBufferView& shaderConstant) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderConstant);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderConstant);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const void* shaderConstant, int size, int offset) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderConstant, size, offset);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderConstant, size, offset);
	}
}


void GraphicsCommandList::NewScratchSpace(size_t hint) {
	ComputeCommandList::NewScratchSpace(hint);
	m_graphicsBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView1D& rwResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView2D& rwResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView3D& rwResource) {
	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWBufferView& rwResource) {
	{
		try {
			m_graphicsBindingManager.Bind(parameter, rwResource);
		}
		catch (std::bad_alloc&) {
			NewScratchSpace(1000);
			m_graphicsBindingManager.Bind(parameter, rwResource);
		}
	}
}


} // namespace gxeng
} // namespace inl