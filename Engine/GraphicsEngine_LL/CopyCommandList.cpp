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


void CopyCommandList::SetResourceState(std::shared_ptr<GenericResource> resource, unsigned subresource, gxapi::eResourceState state) {
	SubresourceId resId{resource, subresource};
	auto iter = m_resourceTransitions.find(resId);
	if (iter == m_resourceTransitions.end()) {
		SubresourceUsageInfo info;
		info.lastState = state;
		info.firstState = state;
		info.multipleStates = false;
		m_resourceTransitions.insert({ std::move(resId), info });
	}
	else {
		const auto& prevLastState = iter->second.lastState;

		ResourceBarrier(gxapi::TransitionBarrier{
							resource->_GetResourcePtr(),
							prevLastState,
							state,
							subresource
		});

		iter->second.lastState = state;
		iter->second.multipleStates = true;
	}
}


BasicCommandList::Decomposition CopyCommandList::Decompose() {
	m_commandList = nullptr;
	return BasicCommandList::Decompose();
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