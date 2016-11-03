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
	m_graphicsApi = gxApi;
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
	CommitRootTables();
}

void GraphicsCommandList::DrawInstanced(unsigned numVertices,
										unsigned startVertex,
										unsigned numInstances,
										unsigned startInstance)
{
	m_commandList->DrawInstanced(numVertices, startVertex, numInstances, startInstance);
	CommitRootTables();
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
	InitRootTables();
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const Texture1DSRV& shaderResource) {
	return BindGraphicsTexture(parameter, shaderResource.GetHandle());
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const Texture2DSRV& shaderResource) {
	return BindGraphicsTexture(parameter, shaderResource.GetHandle());
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const Texture3DSRV& shaderResource) {
	return BindGraphicsTexture(parameter, shaderResource.GetHandle());
}


void GraphicsCommandList::BindGraphicsTexture(BindParameter parameter, gxapi::DescriptorHandle handle) {
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		UpdateRootTableSafe(handle, slot, tableIndex);
	}
	else {
		throw std::invalid_argument("Parameter is not an SRV.");
	}
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const ConstBufferView& shaderConstant) {
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::CBV) {
		m_commandList->SetGraphicsRootConstantBuffer(slot, shaderConstant.GetResource()->GetVirtualAddress());
	}
	else if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		UpdateRootTableSafe(gxapi::DescriptorHandle(), slot, tableIndex);
	}
	else {
		throw std::invalid_argument("Parameter is not a CBV.");
	}
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, const void* shaderConstant, int size, int offset) {
	if (size % 4 != 0) {
		throw std::invalid_argument("Size must be a multiple of 4.");
	}
	assert(m_binder != nullptr);

	int slot;
	int tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex); // may throw out of range

	if (desc.rootParameters[slot].type == gxapi::RootParameterDesc::CONSTANT) {
		assert(desc.rootParameters[slot].As<gxapi::RootParameterDesc::CONSTANT>().numConstants >= unsigned(size + offset) / 4);
		m_commandList->SetGraphicsRootConstants(slot, offset, size / 4, reinterpret_cast<const uint32_t*>(shaderConstant));
	}
	else {
		throw std::invalid_argument("Parameter is not an inline constant.");
	}
}


void GraphicsCommandList::UpdateRootTable(gxapi::DescriptorHandle handle, int rootSignatureSlot, int indexInTable) {
	DescriptorTableState& table = FindRootTable(rootSignatureSlot);

	// if table is committed, duplicate it so that recent drawcalls won't be broken
	if (table.committed) {
		// update handle in advance so that duplicate will copy it instead and we save time
		table.bindings[indexInTable] = handle;
		DuplicateRootTable(table);

		// update table root parameters
		m_commandList->SetGraphicsRootDescriptorTable(rootSignatureSlot, table.reference.Get(0));
	}
	// just update the binding
	else {
		table.bindings[indexInTable] = handle;
		m_graphicsApi->CopyDescriptors(handle, table.reference.Get(indexInTable), 1, gxapi::eDescriptorHeapType::CBV_SRV_UAV);
	}
}


void GraphicsCommandList::DuplicateRootTable(DescriptorTableState& table) {
	uint32_t numDescriptors = table.bindings.size();

	// allocate new space on scratch space
	DescriptorArrayRef space = GetCurrentScratchSpace()->Allocate(numDescriptors);

	// copy old descriptors to new space
	std::vector<gxapi::DescriptorHandle> sourceDescHandles(numDescriptors);
	std::vector<uint32_t> sourceRangeSizes(numDescriptors, 1);
	for (size_t i = 0; i < numDescriptors; ++i) {
		sourceDescHandles[i] = table.bindings[i];
	}

	gxapi::DescriptorHandle destDescHandle = space.Get(0);

	m_graphicsApi->CopyDescriptors(
		sourceDescHandles.size(), sourceDescHandles.data(), sourceRangeSizes.data(),
		1, &destDescHandle, &numDescriptors,
		gxapi::eDescriptorHeapType::CBV_SRV_UAV);

	// update table parameters
	table.committed = false;
	table.reference = space;
}


auto GraphicsCommandList::FindRootTable(int rootSignatureSlot) -> DescriptorTableState& {
	// root table states are already sorted by init
	auto tableIt = std::lower_bound(
		m_rootTableStates.begin(),
		m_rootTableStates.end(),
		rootSignatureSlot,
		[](const DescriptorTableState& table, int slot) { return table.slot < slot; });

	// slot must be valid
	assert(tableIt != m_rootTableStates.end());

	return *tableIt;
}


void GraphicsCommandList::InitRootTables() {
	m_rootTableStates.clear();
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();

	for (size_t slot = 0; slot < desc.rootParameters.size(); slot++) {
		auto& param = desc.rootParameters[slot];
		if (param.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
			auto& ranges = param.As<gxapi::RootParameterDesc::DESCRIPTOR_TABLE>().ranges;

			if (ranges.size() <= 0) {
				continue;
			}

			// if first range is NOT a sampler, non of the ranges are
			// dynamic samplers are not supported, thus the exception
			if (ranges[0].type == gxapi::DescriptorRange::eType::SAMPLER) {
				throw std::runtime_error("Dynamic Samplers are not supported yet.");
			}

			// check if ranges are contiguous and not unbounded
			size_t descriptorCountTotal = 0;
			size_t appendIndex = 0;
			size_t largestIndex = 0;
			for (const auto& range : ranges) {
				size_t rangeOffset;
				descriptorCountTotal += range.numDescriptors;
				if (range.offsetFromTableStart == gxapi::DescriptorRange::OFFSET_APPEND) {
					rangeOffset = appendIndex;
				}
				else {
					rangeOffset = range.offsetFromTableStart;
				}
				largestIndex = std::max(largestIndex, rangeOffset + range.numDescriptors);
				appendIndex = rangeOffset + range.numDescriptors;
			}
			assert(descriptorCountTotal == largestIndex);

			// add record for this table
			m_rootTableStates.push_back({ GetCurrentScratchSpace()->Allocate(descriptorCountTotal), (int)slot });
			m_rootTableStates.back().bindings.resize(descriptorCountTotal);
		}
	}
}


void GraphicsCommandList::CommitRootTables() {
	for (auto& table : m_rootTableStates) {
		table.committed = true;
	}
}


void GraphicsCommandList::RenewRootTables() {
	for (auto& table : m_rootTableStates) {
		DuplicateRootTable(table);
	}
}

void GraphicsCommandList::UpdateRootTableSafe(gxapi::DescriptorHandle handle, int rootSignatureSlot, int indexInTable) {
	try {
		UpdateRootTable(handle, rootSignatureSlot, indexInTable);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		RenewRootTables();
		UpdateRootTable(handle, rootSignatureSlot, indexInTable);
	}
}


} // namespace gxeng
} // namespace inl