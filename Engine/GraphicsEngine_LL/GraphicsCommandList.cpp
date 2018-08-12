#include "GraphicsCommandList.hpp"
#include "VolatileViewHeap.hpp"
#include "MemoryManager.hpp"

namespace inl {
namespace gxeng {


//------------------------------------------------------------------------------
// Basic stuff
//------------------------------------------------------------------------------

GraphicsCommandList::GraphicsCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandListPool& commandListPool,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool,
	MemoryManager& memoryManager,
	VolatileViewHeap& volatileCbvHeap
) :
	ComputeCommandList(gxApi, commandListPool, commandAllocatorPool, scratchSpacePool, memoryManager, volatileCbvHeap, gxapi::eCommandListType::GRAPHICS)
{
	m_commandList = dynamic_cast<gxapi::IGraphicsCommandList*>(GetCommandList());
	m_graphicsBindingManager = BindingManager<gxapi::eCommandListType::GRAPHICS>(m_graphicsApi, m_commandList, &memoryManager, &volatileCbvHeap);
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
void GraphicsCommandList::ClearDepthStencil(const DepthStencilView2D& resource,
	float depth,
	uint8_t stencil,
	size_t numRects,
	gxapi::Rectangle* rects,
	bool clearDepth,
	bool clearStencil)
{
	//ExpectResourceState(resource.GetResource(), gxapi::eResourceState::DEPTH_WRITE);
	m_commandList->ClearDepthStencil(resource.GetHandle(), depth, stencil, numRects, rects, clearDepth, clearStencil);
}

void GraphicsCommandList::ClearRenderTarget(const RenderTargetView2D& resource,
	gxapi::ColorRGBA color,
	size_t numRects,
	gxapi::Rectangle* rects)
{
	ExpectResourceState(resource.GetResource(), gxapi::eResourceState::RENDER_TARGET, resource.GetSubresourceList());
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

	m_performanceCounters.numDrawCalls++;
}

void GraphicsCommandList::DrawInstanced(unsigned numVertices,
	unsigned startVertex,
	unsigned numInstances,
	unsigned startInstance)
{
	m_commandList->DrawInstanced(numVertices, startVertex, numInstances, startInstance);
	m_graphicsBindingManager.CommitDrawCall();

	m_performanceCounters.numDrawCalls++;
}


//------------------------------------------------------------------------------
// Input assembler
//------------------------------------------------------------------------------

void GraphicsCommandList::SetIndexBuffer(const IndexBuffer* resource, bool is32Bit) {
	ExpectResourceState(*resource, gxapi::eResourceState::INDEX_BUFFER, { gxapi::ALL_SUBRESOURCES });
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
		ExpectResourceState(*(resources[i]), gxapi::eResourceState::VERTEX_AND_CONSTANT_BUFFER, { gxapi::ALL_SUBRESOURCES });
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
	const RenderTargetView2D* const* renderTargets,
	const DepthStencilView2D* depthStencil)
{
	auto renderTargetHandles = std::make_unique<gxapi::DescriptorHandle[]>(numRenderTargets);
	for (unsigned i = 0; i < numRenderTargets; ++i) {
		ExpectResourceState(renderTargets[i]->GetResource(), gxapi::eResourceState::RENDER_TARGET, renderTargets[i]->GetSubresourceList());
		renderTargetHandles[i] = renderTargets[i]->GetHandle();
	}

	if (depthStencil) {
		ExpectResourceState(depthStencil->GetResource(), gxapi::eResourceState::DEPTH_WRITE, depthStencil->GetSubresourceList());
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

void GraphicsCommandList::SetGraphicsBinder(const Binder* binder) {
	assert(binder != nullptr);
	try {
		m_graphicsBindingManager.SetBinder(binder);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.SetBinder(binder);
	}
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView1D& shaderResource) {
	ExpectResourceState(
		shaderResource.GetResource(),
		gxapi::eResourceState{ gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE },
		shaderResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView2D& shaderResource) {
	ExpectResourceState(
		shaderResource.GetResource(),
		gxapi::eResourceState{ gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE },
		shaderResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureView3D& shaderResource) {
	ExpectResourceState(
		shaderResource.GetResource(),
		gxapi::eResourceState{ gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE },
		shaderResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const TextureViewCube& shaderResource) {
	ExpectResourceState(
		shaderResource.GetResource(),
		gxapi::eResourceState{ gxapi::eResourceState::PIXEL_SHADER_RESOURCE, gxapi::eResourceState::NON_PIXEL_SHADER_RESOURCE },
		shaderResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const ConstBufferView& shaderConstant) {
	if (dynamic_cast<const PersistentConstBuffer*>(&shaderConstant.GetResource())) {
		m_additionalResources.push_back(shaderConstant.GetResource());
	}

	try {
		m_graphicsBindingManager.Bind(parameter, shaderConstant);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderConstant);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const void* shaderConstant, int size/*, int offset*/) {
	try {
		m_graphicsBindingManager.Bind(parameter, shaderConstant, size/*, offset*/);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, shaderConstant, size/*, offset*/);
	}
}


void GraphicsCommandList::NewScratchSpace(size_t hint) {
	ComputeCommandList::NewScratchSpace(hint);
	m_graphicsBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView1D& rwResource) {
	ExpectResourceState(rwResource.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, rwResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView2D& rwResource) {
	ExpectResourceState(rwResource.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, rwResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWTextureView3D& rwResource) {
	ExpectResourceState(rwResource.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, rwResource.GetSubresourceList());

	try {
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_graphicsBindingManager.Bind(parameter, rwResource);
	}
}

void GraphicsCommandList::BindGraphics(BindParameter parameter, const RWBufferView& rwResource) {
	ExpectResourceState(rwResource.GetResource(), gxapi::eResourceState::UNORDERED_ACCESS, rwResource.GetSubresourceList());

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