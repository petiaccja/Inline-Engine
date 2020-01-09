#pragma once

#include "ICommandList.hpp"

namespace inl::gxapi {


// note: done
class ICommandAllocator {
public:
	virtual ~ICommandAllocator() = default;

	virtual void Reset() = 0;
	virtual eCommandListType GetType() const = 0;
};


} // namespace inl::gxapi
