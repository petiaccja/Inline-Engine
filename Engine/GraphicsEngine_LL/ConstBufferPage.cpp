#include "ConstBufferPage.hpp"

namespace inl {
namespace gxeng {


ConstBufferPage::ConstBufferPage(std::unique_ptr<gxapi::IResource const>&& representedMemory, void* cpuAddress, void* gpuAddress, size_t pageSize) :
	m_representedMemory{std::move(representedMemory)},
	m_cpuAddress(cpuAddress),
	m_gpuAddress(gpuAddress),
	m_pageSize{pageSize},
	m_consumedSize(0),
	m_age(0)
{}


void* ConstBufferPage::GetCpuAddress() const {
	return m_cpuAddress;
}


void* ConstBufferPage::GetGpuAddress() const {
	return m_gpuAddress;
}


} // namespace gxeng
} // namespace inl
