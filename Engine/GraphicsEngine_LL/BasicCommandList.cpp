#include "BasicCommandList.hpp"
#include <iterator>

namespace inl {
namespace gxeng {


BasicCommandList::BasicCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& commandAllocatorPool, ScratchSpacePool& scratchSpacePool, gxapi::eCommandListType type)
	: m_scratchSpacePool(&scratchSpacePool)
{
	// Create a command allocator
	m_commandAllocator = commandAllocatorPool.RequestAllocator(type);

	// Create command list
	gxapi::CommandListDesc desc{ m_commandAllocator.get(), nullptr };
	switch (m_commandAllocator->GetType()) {
		case gxapi::eCommandListType::COPY: m_commandList.reset(gxApi->CreateCopyCommandList(desc)); break;
		case gxapi::eCommandListType::COMPUTE: m_commandList.reset(gxApi->CreateComputeCommandList(desc)); break;
		case gxapi::eCommandListType::GRAPHICS: m_commandList.reset(gxApi->CreateGraphicsCommandList(desc)); break;
		default: assert(false);
	}

	// Create scratch space
	m_scratchSpaces.push_back(m_scratchSpacePool->RequestScratchSpace());
	m_currentScratchSpace = m_scratchSpaces[0].get();
}


BasicCommandList::BasicCommandList(BasicCommandList&& rhs)
	: m_resourceTransitions(std::move(rhs.m_resourceTransitions)),
	m_scratchSpacePool(rhs.m_scratchSpacePool),
	m_scratchSpaces(std::move(rhs.m_scratchSpaces)),
	m_commandAllocator(std::move(rhs.m_commandAllocator)),
	m_commandList(std::move(rhs.m_commandList)),
	m_currentScratchSpace(rhs.m_currentScratchSpace)
{}


BasicCommandList& BasicCommandList::operator=(BasicCommandList&& rhs) {
	m_resourceTransitions = std::move(rhs.m_resourceTransitions);
	m_scratchSpacePool = rhs.m_scratchSpacePool;
	m_scratchSpaces = std::move(rhs.m_scratchSpaces);
	m_commandAllocator = std::move(rhs.m_commandAllocator);
	m_commandList = std::move(rhs.m_commandList);
	m_currentScratchSpace = rhs.m_currentScratchSpace;

	return *this;
}


BasicCommandList::Decomposition BasicCommandList::Decompose() {
	Decomposition decomposition;
	decomposition.commandAllocator = std::move(m_commandAllocator);
	decomposition.commandList = std::move(m_commandList);
	decomposition.usedResources.reserve(m_resourceTransitions.size());

	// Copy the elements of state transition map to vector w/ transforming types.
	for (const auto& v : m_resourceTransitions) {
		decomposition.usedResources.push_back( ResourceUsage{ v.first.resource, v.first.subresource, v.second.firstState, v.second.lastState, v.second.multipleStates } );
	}

	return decomposition;
}




} // namespace gxapi
} // namespace inl