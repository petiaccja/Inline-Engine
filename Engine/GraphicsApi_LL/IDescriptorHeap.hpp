#pragma once

#include "Common.hpp"

#include <cstdint>


namespace inl::gxapi {

// note: done
class IDescriptorHeap {
public:
	virtual ~IDescriptorHeap() = default;

	virtual DescriptorHandle At(size_t index) const = 0;

	virtual DescriptorHeapDesc GetDesc() const = 0;
	virtual uint32_t GetIncrementSize() const = 0;
};


} // namespace inl::gxapi