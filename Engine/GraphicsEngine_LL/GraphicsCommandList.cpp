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
):
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
	DuplicateDescriptors();
}

void GraphicsCommandList::DrawInstanced(unsigned numVertices,
										unsigned startVertex,
										unsigned numInstances,
										unsigned startInstance)
{
	m_commandList->DrawInstanced(numVertices, startVertex, numInstances, startInstance);
	DuplicateDescriptors();
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
	InitializeDescTableStates();
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
		WriteScratchSpace(handle, slot, tableIndex);
	}
	else {
		throw std::invalid_argument("Parameter is not an SRV.");
	}
}


void GraphicsCommandList::BindGraphics(BindParameter parameter, ConstBuffer* shaderConstant) {
	throw std::runtime_error("not implemented");
	assert(m_binder != nullptr);

	int slot, tableIndex;
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();
	m_binder->Translate(parameter, slot, tableIndex);
	const auto& rootParam = desc.rootParameters[slot];

	if (rootParam.type == gxapi::RootParameterDesc::CBV) {
		m_commandList->SetGraphicsRootConstantBuffer(slot, shaderConstant->GetVirtualAddress());
	}
	else if (rootParam.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
		WriteScratchSpace(gxapi::DescriptorHandle(), slot, tableIndex);
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
		assert(desc.rootParameters[slot].As<gxapi::RootParameterDesc::CONSTANT>().numConstants >= (size + offset) / 4);
		m_commandList->SetGraphicsRootConstants(slot, offset, size / 4, reinterpret_cast<const uint32_t*>(shaderConstant));
	}
	else {
		throw std::invalid_argument("Parameter is not an inline constant.");
	}
}


void GraphicsCommandList::WriteScratchSpace(gxapi::DescriptorHandle handle, int slot, int index) {
	assert(
		std::is_sorted(
			m_tableStates.begin(),
			m_tableStates.end(),
			[](const DescriptorTableState& a, const DescriptorTableState& b){return a.slot < b.slot;}
		)
	);

	// find the table with the given slot using binary search
	auto tableIt = std::lower_bound(
		m_tableStates.begin(),
		m_tableStates.end(),
		slot,
		[](const DescriptorTableState& table, int slot){ return table.slot < slot; }
	);
	assert(tableIt != m_tableStates.end());

	assert(tableIt->boundDescriptors.size() > index);
	tableIt->boundDescriptors[index] = handle;

	m_graphicsApi->CopyDescriptors(handle, tableIt->reference.Get(index), 1, gxapi::eDescriptorHeapType::CBV_SRV_UAV);
}


void GraphicsCommandList::DuplicateDescriptors() {
	std::vector<gxapi::DescriptorHandle> srcStarts;
	std::vector<gxapi::DescriptorHandle> dstStarts;
	std::vector<uint32_t> srcLengths;
	std::vector<uint32_t> dstLengths;

	const size_t tableCount = m_tableStates.size();
	srcStarts.reserve(tableCount);
	dstStarts.reserve(tableCount);
	srcLengths.reserve(tableCount);
	dstLengths.reserve(tableCount);

	std::vector<DescriptorTableState> newStates;
	newStates.reserve(m_tableStates.size());

	for (auto& table : m_tableStates) {
		DescriptorTableState newTable(
			GetCurrentScratchSpace()->Allocate(table.reference.Count()),
			false,
			table.slot
		);

		newTable.boundDescriptors.resize(table.boundDescriptors.size());

		for (unsigned i = 0; i < table.boundDescriptors.size(); i++) {
			srcStarts.push_back(table.boundDescriptors[i]);
			srcLengths.push_back(1);
		}
		dstStarts.push_back(newTable.reference.Get(0));
		dstLengths.push_back(newTable.reference.Count());

		newStates.push_back(std::move(newTable));
	}

	m_graphicsApi->CopyDescriptors(
		srcStarts.size(),
		srcStarts.data(),
		srcLengths.data(),
		dstStarts.size(),
		dstStarts.data(),
		dstLengths.data(),
		gxapi::eDescriptorHeapType::CBV_SRV_UAV
	);

	m_tableStates = std::move(newStates);
}


void GraphicsCommandList::InitializeDescTableStates() {
	m_tableStates.clear();
	const gxapi::RootSignatureDesc& desc = m_binder->GetRootSignatureDesc();

	for (size_t slot = 0; slot < desc.rootParameters.size(); slot++) {
		auto& param = desc.rootParameters[slot];
		if (param.type == gxapi::RootParameterDesc::DESCRIPTOR_TABLE) {
			auto& ranges = param.As<gxapi::RootParameterDesc::DESCRIPTOR_TABLE>().ranges;
			assert(ranges.size() >= 1);

			// NOTE: If first range is not a sampler, than no range is a sampler in this table
			if(ranges[0].type == gxapi::DescriptorRange::eType::SAMPLER) {
				throw std::runtime_error("Dynamic Samplers are not supported at this time");
			}

			// =========================================================
			// Assertion: ranges are contiguous
#ifdef _DEBUG
			if (ranges.size() > 1) {
				for (auto& curr : ranges) {
					assert(curr.offsetFromTableStart == gxapi::DescriptorRange::OFFSET_APPEND);
				}
			}
#endif // _DEBUG
			// =========================================================

			unsigned tableSize = 0;
			for (auto& range : ranges) {
				tableSize += range.numDescriptors;
			}

			m_tableStates.push_back(DescriptorTableState(GetCurrentScratchSpace()->Allocate(tableSize), false, slot));
			m_tableStates.back().boundDescriptors.resize(tableSize);
		}
	}
}


} // namespace gxeng
} // namespace inl