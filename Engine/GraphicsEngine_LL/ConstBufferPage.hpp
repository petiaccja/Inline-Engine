#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {


class ConstBufferPage {
	friend class ConstBufferManager;
public:
	ConstBufferPage() = default;
	ConstBufferPage(std::unique_ptr<gxapi::IResource const>&& representedMemory, void* cpuAddress, void* gpuAddress, size_t pageSize);
	ConstBufferPage(ConstBufferPage&&) = default;
	ConstBufferPage& operator=(ConstBufferPage&&) = default;

	void* GetCpuAddress() const;
	void* GetGpuAddress() const;

protected:
	std::unique_ptr<gxapi::IResource const> m_representedMemory;
	void* const m_cpuAddress;
	void* const m_gpuAddress;
	const size_t m_pageSize;
	size_t m_consumedSize;
	int m_age;
};


} // namespace gxeng
} // namespace inl
