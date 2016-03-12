#pragma once


#include <cstdint>


namespace inl {
namespace gxapi {


class ICommandList;
class IFence;


enum class eCommandQueueType {
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
	void ExecuteCommandLists(ICommandList* const* commandLists);

	void Signal(IFence* fence, uint64_t value);
	void Wait(IFence* fence, uint64_t value);

	eCommandQueueType GetType() const;
	eCommandQueuePriority GetPriority() const;
	bool IsGPUTimeoutEnabled() const;
};

} // namespace gxapi
} // namespace inl
