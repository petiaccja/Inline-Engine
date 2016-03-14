#pragma once

#include "DescriptorHandle.hpp"
#include "Common.hpp"

#include <cstdint>


namespace inl {
namespace gxapi {

// note: done
class IDescriptorHeap {
public: 
	virtual ~IDescriptorHeap() = default;

	virtual DescriptorHandle At(size_t index) const = 0;

	virtual size_t GetNumDescriptors() const = 0;
	virtual eDesriptorHeapType GetType() const = 0;
	virtual bool IsShaderVisible() const = 0;
};


} // namespace gxapi
} // namespace inl