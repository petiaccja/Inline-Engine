#include "BasicCommandList.hpp"

namespace inl {
namespace gxeng {


BasicCommandList::BasicCommandList(CommandAllocatorPool& cmdAllocatorPool, gxapi::eCommandListType type)
	: m_commandAllocator(nullptr, AllocDeleter(cmdAllocatorPool))
{
	gxapi::IGraphicsApi* gxApi = cmdAllocatorPool.GetGraphicsApi();

	m_commandAllocator.reset(cmdAllocatorPool.RequestAllocator(type));

	gxapi::CommandListDesc desc{m_commandAllocator.get(), nullptr};
	m_commandList.reset(gxApi->CreateGraphicsCommandList(desc));

}

BasicCommandList::~BasicCommandList() {
	
}


void BasicCommandList::UseResource(GenericResource* resource) {
	
}


}
}