#pragma once

#include "HostDescHeap.hpp"

namespace inl {
namespace gxeng {

#if 0
class VolatileViewHeap {
public:
	VolatileViewHeap();

	DescriptorReference Allocate();

protected:
	inline StackDescHeap* GetCurrentScratchSpace();

private:
	CbvSrvUavHeap m_heap;
};
#endif

} // namespace gxeng
} // namespace inl
