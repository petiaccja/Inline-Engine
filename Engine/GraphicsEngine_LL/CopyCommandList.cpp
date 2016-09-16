#include "CopyCommandList.hpp"

namespace inl {
namespace gxeng {


CopyCommandList::CopyCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool)
	: BasicCommandList(gxApi, commandAllocatorPool, scratchSpacePool, gxapi::eCommandListType::COPY)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type)
	: BasicCommandList(gxApi, commandAllocatorPool, scratchSpacePool, type)
{
	m_commandList = dynamic_cast<gxapi::ICopyCommandList*>(GetCommandList());
}


CopyCommandList::CopyCommandList(CopyCommandList&& rhs)
	: BasicCommandList(std::move(rhs)),
	m_commandList(rhs.m_commandList) 
{
	rhs.m_commandList = nullptr;
}


CopyCommandList& CopyCommandList::operator=(CopyCommandList&& rhs) {
	BasicCommandList::operator=(std::move(rhs));
	m_commandList = rhs.m_commandList;
	rhs.m_commandList = nullptr;

	return *this;
}


void CopyCommandList::RegisterResourceTransition(const SubresourceID& subresource, gxapi::eResourceState targetState) {
	auto iter = m_resourceTransitions.find(subresource);
	if (iter == m_resourceTransitions.end()) {
		StateTransitionRegister reg;
		reg.lastTargetState = targetState;
		reg.firstTargetState = targetState;
		reg.multipleTransition = false;
		m_resourceTransitions.insert({subresource, reg});
	}
	else {
		const auto& prevTargetState = iter->second.lastTargetState;

		ResourceBarrier(
			gxapi::TransitionBarrier{
				subresource.resource,
				prevTargetState,
				targetState,
				subresource.subresource
			}
		);

		iter->second.lastTargetState = targetState;
		iter->second.multipleTransition = true;
	}
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
}



// Command list state
void CopyCommandList::ResetState(gxapi::IPipelineState* newState) {
	m_commandList->ResetState(newState);
}


void CopyCommandList::CopyBuffer(GenericResource * dst, size_t dstOffset, GenericResource * src, size_t srcOffset, size_t numBytes) {
	m_commandList->CopyBuffer(dst->_GetResourcePtr(), dstOffset, src->_GetResourcePtr(), srcOffset, numBytes);
}



// resource copy
// TODO


// barriers
void CopyCommandList::ResourceBarrier(unsigned numBarriers, gxapi::ResourceBarrier* barriers) {
	m_commandList->ResourceBarrier(numBarriers, barriers);
}




} // namespace gxeng
} // namespace inl