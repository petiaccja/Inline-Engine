#pragma once

#include "../BaseLibrary/RingBuffer.hpp"
#include "../GraphicsApi_LL/IFence.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "DisposableConstBuffer.hpp"
#include "ConstBufferPage.hpp"

#include <mutex>

namespace inl {
namespace gxeng {


class ConstBufferManager {
public:
	ConstBufferManager(gxapi::IGraphicsApi* graphicsApi);

	DisposableConstBuffer GetDisposableBuffer(size_t size);
	void FrameCompleted();

protected:
	exc::RingBuffer<ConstBufferPage> m_pages;
	gxapi::IGraphicsApi* m_graphicsApi;
	short m_backBufferCount;
	std::mutex m_mutex;

protected:
	// From ( https://msdn.microsoft.com/en-us/library/windows/desktop/dn899216%28v=vs.85%29.aspx )
	// "Linear subresource copying must be aligned to 512 bytes (with the row pitch aligned to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT bytes)."
	static constexpr size_t ALIGNEMENT = 256;
	static constexpr size_t PAGE_SIZE = 64*1024;

	static size_t AlignUp(size_t value, size_t alignement);
protected:
	ConstBufferPage CreatePage();
};


} // namespace gxeng
} // namespace inl
