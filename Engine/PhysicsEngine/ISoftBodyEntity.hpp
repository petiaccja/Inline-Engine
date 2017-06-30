#pragma once
#include <stdint.h>

namespace inl::physics {

class ISoftBodyEntity
{
public:
	virtual void* GetUserPointer() = 0;
	virtual uint64_t GetCollisionGroup() const = 0;
};

} // namspace physics