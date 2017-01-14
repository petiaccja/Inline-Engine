#include "VolatileViewHeap.hpp"


namespace inl {
namespace gxeng {

#if 0
VolatileViewHeap::VolatileViewHeap()
{
}



DescriptorArrayRef VolatileViewHeap::Allocate(size_t descCount) {
	
	try {
		return GetCurrentScratchSpace()->Allocate(descCount);
	}
	catch (std::bad_alloc&) {
		m_scratchSpaceRequestor.NewScratchSpace(100);
		return GetCurrentScratchSpace()->Allocate(descCount);
	}
}


VolatileViewHeap::Decomposition VolatileViewHeap::Decompose() {
	Decomposition result;
	result.scratchSpaces = m_scratchSpaceRequestor.TakePointers();

	return result;
}


StackDescHeap* VolatileViewHeap::GetCurrentScratchSpace() {
	return m_scratchSpaceRequestor.GetCurrentScratchSpace();
}
#endif

} // namespace gxeng
} // namespace inl
