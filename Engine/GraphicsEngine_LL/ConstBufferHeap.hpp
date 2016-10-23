#pragma once

#include "GpuBuffer.hpp"
#include "PipelineEventListener.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"
#include "../BaseLibrary/RingBuffer.hpp"
#include "../BaseLibrary/ScalarLiterals.hpp"

#include <memory>

namespace inl {
namespace gxeng {

using namespace exc::prefix;

class MemoryManager;

class ConstantBufferHeap : public PipelineEventListener {
protected:
	class ConstBufferPage {
	public:
		ConstBufferPage() = default;
		ConstBufferPage(std::unique_ptr<gxapi::IResource>&& representedMemory, void* cpuAddress, void* gpuAddress, size_t pageSize, size_t ownerFrameID) :
			m_representedMemory(std::move(representedMemory)),
			m_cpuAddress(cpuAddress),
			m_gpuAddress(gpuAddress),
			m_pageSize(pageSize),
			m_consumedSize(0),
			m_ownerFrameID(ownerFrameID)
		{}
		ConstBufferPage(ConstBufferPage&&) = default;
		ConstBufferPage& operator=(ConstBufferPage&&) = default;

		std::unique_ptr<gxapi::IResource> m_representedMemory;
		void* const m_cpuAddress;
		void* const m_gpuAddress;
		const size_t m_pageSize;
		size_t m_consumedSize;
		size_t m_ownerFrameID;
	};

public:
	ConstantBufferHeap(gxapi::IGraphicsApi* graphicsApi);

	VolatileConstBuffer CreateVolatileBuffer(void* data, size_t dataSize);
	PersistentConstBuffer CreatePersistentBuffer(void* data, size_t dataSize);

	void OnFrameBeginDevice(uint64_t frameId) override;
	void OnFrameBeginHost(uint64_t frameId) override;
	void OnFrameCompleteDevice(uint64_t frameId) override;
	void OnFrameCompleteHost(uint64_t frameId) override;

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	exc::RingBuffer<ConstBufferPage> m_largePages;
	exc::RingBuffer<ConstBufferPage> m_pages;
	std::mutex m_mutex;

	size_t m_currFrameID = 1;
	size_t m_lastFinishedFrameID = 0;

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
	//static constexpr uint8_t CMDLIST_FINISH_FRAME_COUNT = 2;

	static size_t AlignUp(size_t value, size_t alignement);

protected:
	ConstBufferPage CreatePage();
	ConstBufferPage CreateLargePage(size_t fittingSize);
	bool HasBecomeAvailable(const ConstBufferPage& page);
};

} // namespace gxeng
} // namespace inl
