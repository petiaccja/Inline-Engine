#pragma once

#include "ConstBufferPage.hpp"

namespace inl {
namespace gxeng {


class DisposableConstBuffer {
	friend class ConstBufferManager;
private:
	DisposableConstBuffer(ConstBufferPage* page, size_t offsetOnPage, size_t size);

public:
	DisposableConstBuffer(DisposableConstBuffer&&) = default;

	DisposableConstBuffer(DisposableConstBuffer&) = delete;
	DisposableConstBuffer& operator=(DisposableConstBuffer&) = delete;

	void UploadData(void* data, size_t size);
	void* GetGpuAddress();

protected:
	ConstBufferPage* m_page;
	size_t m_offsetOnPage;
	size_t m_size;
};


} // namespace gxeng
} // namespace inl
