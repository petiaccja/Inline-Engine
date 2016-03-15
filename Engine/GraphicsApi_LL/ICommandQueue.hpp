#pragma once


#include <cstdint>


namespace inl {
namespace gxapi {


class ICommandList;
class IFence;


// note: done
class ICommandQueue {
public:
	virtual ~ICommandQueue() = default;

	virtual void ExecuteCommandLists(uint32_t numCommandLists, ICommandList* const* commandLists) = 0;

	virtual void Signal(IFence* fence, uint64_t value) = 0;
	virtual void Wait(IFence* fence, uint64_t value) = 0;

	virtual CommandQueueDesc GetDesc() const = 0;
};

} // namespace gxapi
} // namespace inl
