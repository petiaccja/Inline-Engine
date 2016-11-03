#include "BasicCommandList.hpp"
#include <iterator>
#include "GraphicsApi_D3D12/CommandList.hpp"

namespace inl {
namespace gxeng {


BasicCommandList::BasicCommandList(gxapi::IGraphicsApi* gxApi,
								   CommandAllocatorPool& commandAllocatorPool,
								   ScratchSpacePool& scratchSpacePool,
								   gxapi::eCommandListType type
) :
	m_scratchSpacePool(&scratchSpacePool)
{
	// Create a command allocator
	m_commandAllocator = commandAllocatorPool.RequestAllocator(type);

	// Create command list
	gxapi::CommandListDesc desc{ m_commandAllocator.get(), nullptr };
	switch (type) {
		case gxapi::eCommandListType::COPY: m_commandList.reset(gxApi->CreateCopyCommandList(desc)); break;
		case gxapi::eCommandListType::COMPUTE: m_commandList.reset(gxApi->CreateComputeCommandList(desc)); break;
		case gxapi::eCommandListType::GRAPHICS: m_commandList.reset(gxApi->CreateGraphicsCommandList(desc)); break;
		default: assert(false);
	}

	// Create scratch space
	if (type == gxapi::eCommandListType::COMPUTE || type == gxapi::eCommandListType::GRAPHICS) {
		NewScratchSpace(1000);
	}
	else {
		m_currentScratchSpace = nullptr;
	}
}


StackDescHeap* BasicCommandList::GetCurrentScratchSpace() {
	return m_currentScratchSpace;
}

void BasicCommandList::NewScratchSpace(size_t sizeHint) {
	gxapi::IComputeCommandList* cuCommandList = dynamic_cast<gxapi::IComputeCommandList*>(m_commandList.get());
	if (cuCommandList == nullptr) {
		return;
	}

	ScratchSpacePtr newScratchSpace = m_scratchSpacePool->RequestScratchSpace();
	m_scratchSpaces.push_back(std::move(newScratchSpace));
	m_currentScratchSpace = m_scratchSpaces.back().get();

	gxapi::IDescriptorHeap* descHeap = m_currentScratchSpace->GetHeap();
	cuCommandList->SetDescriptorHeaps(&descHeap, 1);
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
	decomposition.scratchSpaces = std::move(m_scratchSpaces);
	decomposition.usedResources.reserve(m_resourceTransitions.size());

	// Copy the elements of state transition map to vector w/ transforming types.
	for (const auto& v : m_resourceTransitions) {
		decomposition.usedResources.push_back(ResourceUsage{ std::move(v.first.resource), v.first.subresource, v.second.firstState, v.second.lastState, v.second.multipleStates });
	}

	return decomposition;
}




} // namespace gxapi
} // namespace inl