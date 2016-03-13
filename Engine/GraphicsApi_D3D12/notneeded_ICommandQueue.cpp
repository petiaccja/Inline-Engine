#include "../GraphicsApi_LL/ICommandQueue.hpp"

#include "../GraphicsApi_LL/IFence.hpp"
#include <d3d12.h>

namespace inl {
namespace gxapi {


void ICommandQueue::ExecuteCommandLists(ICommandList* const* commandLists)
{

}

void ICommandQueue::Signal(IFence* fence, uint64_t value)
{
	fence->
}

void ICommandQueue::Wait(IFence* fence, uint64_t value)
{

}

eCommandQueueType ICommandQueue::GetType() const
{

}

eCommandQueuePriority ICommandQueue::GetPriority() const
{
	
}

bool ICommandQueue::IsGPUTimeoutEnabled() const
{
	

}


}
}
