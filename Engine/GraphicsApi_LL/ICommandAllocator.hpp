#pragma once

#include "ICommandList.hpp"

namespace inl {
namespace gxapi {


// note: done
class ICommandAllocator {
public:
	virtual void Reset() = 0;

};


} // namespace gxapi
} // namespace inl
