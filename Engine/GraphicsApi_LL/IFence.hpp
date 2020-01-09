#pragma once

#include <cstdint>
#include <limits>

namespace inl::gxapi {

class IFence {
public:
	virtual ~IFence() = default;

	virtual uint64_t Fetch() const = 0;
	virtual void Signal(uint64_t value) = 0;
	virtual void Wait(uint64_t value, uint64_t timeoutMillis = FOREVER) const = 0;
	virtual void WaitAny(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis = FOREVER) const = 0;
	virtual void WaitAll(const IFence** fences, uint64_t* values, size_t count, uint64_t timeoutMillis = FOREVER) const = 0;

	static constexpr uint64_t FOREVER = std::numeric_limits<uint64_t>::max();
};

} // namespace gxapi
