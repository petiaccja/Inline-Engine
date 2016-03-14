#pragma once


#include <cstdint>


namespace inl {
namespace gxapi {


class ICommandList;
class IFence;


enum class eCommandListType {
	COPY,
	COMPUTE,
	GRAPHICS,
};


enum class eCommandQueuePriority {
	NORMAL,
	HIGH,
};


// note: done
class ICommandQueue {
public:
	virtual ~ICommandQueue() = default;

	virtual void ExecuteCommandLists(uint32_t numCommandLists, ICommandList* const* commandLists) = 0;

	virtual void Signal(IFence* fence, uint64_t value) = 0;
	virtual void Wait(IFence* fence, uint64_t value) = 0;

	virtual eCommandListType GetType() const = 0;
	virtual eCommandQueuePriority GetPriority() const = 0;
	virtual bool IsGPUTimeoutEnabled() const = 0;
};

} // namespace gxapi
} // namespace inl
