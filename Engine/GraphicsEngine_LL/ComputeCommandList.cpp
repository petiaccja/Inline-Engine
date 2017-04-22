#include "ComputeCommandList.hpp"
#include "MemoryManager.hpp"
#include "VolatileViewHeap.hpp"


namespace inl {
namespace gxeng {

//------------------------------------------------------------------------------
// Basic stuff
//------------------------------------------------------------------------------

ComputeCommandList::ComputeCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool,
	MemoryManager& memoryManager,
	VolatileViewHeap& volatileCbvHeap
) :
	CopyCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::COMPUTE)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(GetCommandList());

	m_computeBindingManager = BindingManager<gxapi::eCommandListType::COMPUTE>(m_graphicsApi, m_commandList, &memoryManager, &volatileCbvHeap);
	m_computeBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
}

ComputeCommandList::ComputeCommandList(
	gxapi::IGraphicsApi* gxApi,
	CommandAllocatorPool& commandAllocatorPool,
	ScratchSpacePool& scratchSpacePool,
	MemoryManager& memoryManager,
	VolatileViewHeap& volatileCbvHeap,
	gxapi::eCommandListType type
) :
	CopyCommandList(gxApi, commandAllocatorPool, scratchSpacePool, type)
{
	m_commandList = dynamic_cast<gxapi::IComputeCommandList*>(GetCommandList());

	m_computeBindingManager = BindingManager<gxapi::eCommandListType::COMPUTE>(m_graphicsApi, m_commandList, &memoryManager, &volatileCbvHeap);
	m_computeBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
}


ComputeCommandList::ComputeCommandList(ComputeCommandList&& rhs)
	: CopyCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList)
{
	rhs.m_commandList = nullptr;
}


ComputeCommandList& ComputeCommandList::operator=(ComputeCommandList&& rhs) {
	CopyCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


BasicCommandList::Decomposition ComputeCommandList::Decompose() {
	m_commandList = nullptr;

	return CopyCommandList::Decompose();
}


//------------------------------------------------------------------------------
// Draw
//------------------------------------------------------------------------------
void ComputeCommandList::Dispatch(size_t numThreadGroupsX, size_t numThreadGroupsY, size_t numThreadGroupsZ) {
	m_commandList->Dispatch(numThreadGroupsX, numThreadGroupsY, numThreadGroupsZ);
}


//------------------------------------------------------------------------------
// Command list state
//------------------------------------------------------------------------------
void ComputeCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_commandList->ResetState(newState);
}

void ComputeCommandList::SetPipelineState(gxapi::IPipelineState* pipelineState) {
	m_commandList->SetPipelineState(pipelineState);
}


//------------------------------------------------------------------------------
// Set compute root signature stuff
//------------------------------------------------------------------------------
void ComputeCommandList::SetComputeBinder(Binder* binder) {
	assert(binder != nullptr);
	m_computeBindingManager.SetBinder(binder);
}


void ComputeCommandList::BindCompute(BindParameter parameter, const TextureView1D& shaderResource) {
	try {
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const TextureView2D& shaderResource) {
	try {
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const TextureView3D& shaderResource) {
	try {
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, shaderResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const ConstBufferView& shaderConstant) {
	try {
		m_computeBindingManager.Bind(parameter, shaderConstant);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, shaderConstant);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const void* shaderConstant, int size/*, int offset*/) {
	try {
		m_computeBindingManager.Bind(parameter, shaderConstant, size/*, offset*/);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, shaderConstant, size/*, offset*/);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const RWTextureView1D& rwResource) {
	try {
		m_computeBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, rwResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const RWTextureView2D& rwResource) {
	try {
		m_computeBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, rwResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const RWTextureView3D& rwResource) {
	try {
		m_computeBindingManager.Bind(parameter, rwResource);
	}
	catch (std::bad_alloc&) {
		NewScratchSpace(1000);
		m_computeBindingManager.Bind(parameter, rwResource);
	}
}

void ComputeCommandList::BindCompute(BindParameter parameter, const RWBufferView& rwResource) {
	{
		try {
			m_computeBindingManager.Bind(parameter, rwResource);
		}
		catch (std::bad_alloc&) {
			NewScratchSpace(1000);
			m_computeBindingManager.Bind(parameter, rwResource);
		}
	}
}


void ComputeCommandList::NewScratchSpace(size_t hint) {
	BasicCommandList::NewScratchSpace(hint);
	m_computeBindingManager.SetDescriptorHeap(GetCurrentScratchSpace());
}


//------------------------------------------------------------------------------
// UAV barrier
//------------------------------------------------------------------------------
void ComputeCommandList::UAVBarrier(const MemoryObject& memoryObject) {
	m_commandList->ResourceBarrier(gxapi::UavBarrier(memoryObject._GetResourcePtr()));
}


} // namespace gxeng
} // namespace inl
