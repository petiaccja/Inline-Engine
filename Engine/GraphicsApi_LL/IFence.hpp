#pragma once

#include <cstdint>
#include <limits>

namespace inl {
namespace gxapi {

class IFence {
public:
	virtual ~IFence() = default;

	virtual uint64_t Fetch() = 0;
	virtual void Signal(uint64_t value) = 0;
	virtual void Wait(uint64_t value, uint64_t timeoutMillis = FOREVER) = 0;

	static constexpr uint64_t FOREVER = std::numeric_limits<uint64_t>::max();
};

} // namespace gxapi
} // namespace inl
