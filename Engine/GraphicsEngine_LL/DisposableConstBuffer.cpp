#include "DisposableConstBuffer.hpp"

#include "../GraphicsApi_LL/Exception.hpp"

namespace inl {
namespace gxeng {

DisposableConstBuffer::DisposableConstBuffer(ConstBufferPage* page, size_t offsetOnPage, size_t size) :
	m_page{page},
	m_offsetOnPage{offsetOnPage},
	m_size{size}
{}

void DisposableConstBuffer::UploadData(const void* data, size_t size) {
	if (size > m_size) {
		throw inl::gxapi::InvalidArgument("Data given to be uploaded exceeds size of buffer. "
			"Data size: " + std::to_string(size) + ". "
			"Buffer size: " + std::to_string(m_size) + ".");
	}

	uint8_t* ptr = (uint8_t*)(m_page->GetCpuAddress());
	memcpy(ptr + m_offsetOnPage, data, size);
}

void* DisposableConstBuffer::GetGpuAddress() {
	return m_page->GetGpuAddress();
}


} // namespace gxeng
} // namespace inl
