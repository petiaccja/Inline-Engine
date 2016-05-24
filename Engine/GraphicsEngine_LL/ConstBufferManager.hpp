#pragma once

#include "../BaseLibrary/ScalarLiterals.hpp"
#include "../BaseLibrary/RingBuffer.hpp"
#include "../GraphicsApi_LL/IFence.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "DisposableConstBuffer.hpp"
#include "ConstBufferPage.hpp"

#include <mutex>

namespace inl {
namespace gxeng {

using namespace exc::prefix;

class ConstBufferManager {
public:
	ConstBufferManager(gxapi::IGraphicsApi* graphicsApi);

	DisposableConstBuffer GetDisposableBuffer(size_t size);
	void FrameCompleted();

protected:
	exc::RingBuffer<ConstBufferPage> m_largePages;
	exc::RingBuffer<ConstBufferPage> m_pages;
	gxapi::IGraphicsApi* m_graphicsApi;
	std::mutex m_mutex;

protected:
	// From ( https://msdn.microsoft.com/en-us/library/windows/desktop/dn899216%28v=vs.85%29.aspx )
	// "Linear subresource copying must be aligned to 512 bytes
	// (with the row pitch aligned to D3D12_TEXTURE_DATA_PITCH_ALIGNMENT bytes)."
	static constexpr size_t ALIGNEMENT = 512;
	static constexpr size_t PAGE_SIZE = 64_Ki;

	static constexpr size_t MAX_PERMANENT_LARGE_PAGE_COUNT = 5;


	// The number of frames needed to be completed for a
	// command list to get from being assembled to being finished
	// Expalnation: ("comp" means how many frames are completed)
	// Frames:      |   0 comp   |   1 comp  |   2 comp   |
	//         ...  |  assembly  |  process  |  finished  |  ...
	static constexpr uint8_t CMDLIST_FINISH_FRAME_COUNT = 2;


	static size_t AlignUp(size_t value, size_t alignement);

protected:
	ConstBufferPage CreatePage();
	ConstBufferPage CreateLargePage(size_t fittingSize);
	bool HasBecomeAvailable(const ConstBufferPage& page);
};


} // namespace gxeng
} // namespace inl
