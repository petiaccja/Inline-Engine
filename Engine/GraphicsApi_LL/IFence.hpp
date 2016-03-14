#pragma once

#include <cstdint>

namespace inl {
namespace gxapi {

class IFence {
public:
	virtual ~IFence() = default;

	virtual uint64_t Fetch() = 0;
	virtual void Signal(uint64_t value) = 0;
};

} // namespace gxapi
} // namespace inl
