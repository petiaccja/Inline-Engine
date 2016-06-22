#include "BasicCommandList.hpp"

namespace inl {
namespace gxeng {


BasicCommandList::BasicCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type)
	: m_commandAllocator(nullptr, AllocDeleter(cmdAllocatorPool)), m_commandAllocatorPool(&cmdAllocatorPool)
{
	gxapi::IGraphicsApi* gxApi = cmdAllocatorPool.GetGraphicsApi();

	m_commandAllocator.reset(cmdAllocatorPool.RequestAllocator(type));

	gxapi::CommandListDesc desc{ m_commandAllocator.get(), nullptr };
	m_commandList.reset(gxApi->CreateGraphicsCommandList(desc));
}


BasicCommandList::BasicCommandList(BasicCommandList&& rhs)
	: m_commandAllocator(std::move(rhs.m_commandAllocator)),
	m_commandList(std::move(rhs.m_commandList)),
	m_usedResources(std::move(m_usedResources))
{}


BasicCommandList& BasicCommandList::operator=(BasicCommandList&& rhs) {
	m_commandAllocator = std::move(rhs.m_commandAllocator);
	m_commandList = std::move(rhs.m_commandList);
	m_usedResources = std::move(m_usedResources);

	return *this;
}


void BasicCommandList::UseResource(GenericBuffer* resource) {
	m_usedResources.push_back(resource);
}


BasicCommandList::Decomposition BasicCommandList::Decompose() {
	Decomposition decomposition;
	decomposition.commandAllocatorPool = m_commandAllocatorPool;
	decomposition.commandAllocator = m_commandAllocator.release();
	decomposition.commandList = m_commandList.release();
	decomposition.usedResources = std::move(m_usedResources);

	return decomposition;
}




} // namespace gxapi
} // namespace inl