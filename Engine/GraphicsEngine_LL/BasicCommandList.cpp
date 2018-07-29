#include "BasicCommandList.hpp"
#include <iterator>
#include "GraphicsApi_D3D12/CommandList.hpp"

namespace inl {
namespace gxeng {


BasicCommandList::BasicCommandList(gxapi::IGraphicsApi* gxApi,
								   CommandListPool& commandListPool,
								   CommandAllocatorPool& commandAllocatorPool,
								   ScratchSpacePool& scratchSpacePool,
								   gxapi::eCommandListType type
) :
	m_scratchSpacePool(&scratchSpacePool)
{
	// Set gxapi
	m_graphicsApi = gxApi;

	// Create a command allocator
	m_commandAllocator = commandAllocatorPool.RequestAllocator(type);

	// Create command list
	gxapi::CommandListDesc desc{ m_commandAllocator.get(), nullptr };
	m_commandList = commandListPool.RequestList(type, m_commandAllocator.get());

	// Create scratch space
	if (type == gxapi::eCommandListType::COMPUTE || type == gxapi::eCommandListType::GRAPHICS) {
		NewScratchSpace(1000);
	}
	else {
		m_currentScratchSpace = nullptr;
	}
}


BasicCommandList::~BasicCommandList() {
	// If the command list was not decomposed, we would like to close it before deleting it.
	if (m_commandList) {
		dynamic_cast<gxapi::ICopyCommandList*>(m_commandList.get())->Close();
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

	assert(m_scratchSpacePool != nullptr);
	ScratchSpacePtr newScratchSpace = m_scratchSpacePool->RequestScratchSpace();
	m_scratchSpaces.push_back(std::move(newScratchSpace));
	m_currentScratchSpace = m_scratchSpaces.back().get();

	gxapi::IDescriptorHeap* descHeap = m_currentScratchSpace->GetHeap();
	cuCommandList->SetDescriptorHeaps(&descHeap, 1);
}


BasicCommandList::BasicCommandList(BasicCommandList&& rhs)
	: m_resourceTransitions(std::move(rhs.m_resourceTransitions)),
	m_scratchSpacePool(rhs.m_scratchSpacePool),
	m_commandAllocator(std::move(rhs.m_commandAllocator)),
	m_commandList(std::move(rhs.m_commandList)),
	m_scratchSpaces(std::move(rhs.m_scratchSpaces)),
	m_currentScratchSpace(rhs.m_currentScratchSpace)
{}


BasicCommandList& BasicCommandList::operator=(BasicCommandList&& rhs) {
	m_resourceTransitions = std::move(rhs.m_resourceTransitions);
	m_scratchSpacePool = rhs.m_scratchSpacePool;
	m_commandAllocator = std::move(rhs.m_commandAllocator);
	m_commandList = std::move(rhs.m_commandList);
	m_scratchSpaces = std::move(rhs.m_scratchSpaces);
	m_currentScratchSpace = rhs.m_currentScratchSpace;

	return *this;
}


BasicCommandList::Decomposition BasicCommandList::Decompose() {
	Decomposition decomposition;
	decomposition.commandAllocator = std::move(m_commandAllocator);
	decomposition.commandList = std::move(m_commandList);
	decomposition.scratchSpaces = std::move(m_scratchSpaces);
	decomposition.usedResources.reserve(m_resourceTransitions.size());
	decomposition.additionalResources = std::move(m_additionalResources);

	// Copy the elements of state transition map to vector w/ transforming types.
	for (const auto& v : m_resourceTransitions) {
		decomposition.usedResources.push_back(ResourceUsage{ std::move(v.first.resource), v.first.subresource, v.second.firstState, v.second.lastState, v.second.multipleStates });
	}

	return decomposition;
}


void BasicCommandList::BeginDebuggerEvent(const std::string& name) {
	m_commandList->BeginDebuggerEvent(name);
}

void BasicCommandList::EndDebuggerEvent() {
	m_commandList->EndDebuggerEvent();
}


void BasicCommandList::SetName(const std::string& name) {
	m_commandList->SetName(name.c_str());
}

void BasicCommandList::SetName(const char* name) {
	m_commandList->SetName(name);
}


} // namespace gxapi
} // namespace inl