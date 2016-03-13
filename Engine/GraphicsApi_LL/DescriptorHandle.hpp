#pragma once

namespace inl {
namespace gxapi {


// todo: operator++, += for traversing heap like iterator?
//		ctor as obj(Heap*, index)
struct DescriptorHandle {
public:
	DescriptorHandle() {
		cpuAddress = nullptr;
		gpuAddress = nullptr;
	}
	DescriptorHandle(const DescriptorHandle&) = default;
	DescriptorHandle& operator=(const DescriptorHandle&) = default;

	bool operator==(const DescriptorHandle& rhs) const {
		return cpuAddress == rhs.cpuAddress && gpuAddress == rhs.gpuAddress;
	}
	bool operator!=(const DescriptorHandle& rhs) const {
		return !(*this == rhs);
	}
public:
	void* cpuAddress;
	void* gpuAddress;
};


} // namespace gxapi
} // namespace inl
