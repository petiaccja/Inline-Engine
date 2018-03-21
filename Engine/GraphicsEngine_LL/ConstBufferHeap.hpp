#pragma once

#include "MemoryObject.hpp"
#include "PipelineEventListener.hpp"
#include "BufferHeap.hpp"

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/IResource.hpp"
#include "../BaseLibrary/RingBuffer.hpp"
#include "../BaseLibrary/ScalarLiterals.hpp"

#include <memory>
#include <mutex>

namespace inl {
namespace gxeng {

class MemoryManager;

class ConstantBufferHeap : public PipelineEventListener, public BufferHeap {
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
		uint64_t m_ownerFrameID;
	};

public:
	ConstantBufferHeap(gxapi::IGraphicsApi* graphicsApi);

	VolatileConstBuffer CreateVolatileConstBuffer(const void* data, uint32_t dataSize) override;
	PersistentConstBuffer CreatePersistentConstBuffer(const void* data, uint32_t dataSize) override;

	void OnFrameBeginDevice(uint64_t frameId) override;
	void OnFrameBeginHost(uint64_t frameId) override;
	void OnFrameBeginAwait(uint64_t frameId) override {};
	void OnFrameCompleteDevice(uint64_t frameId) override;
	void OnFrameCompleteHost(uint64_t frameId) override;

protected:
	gxapi::IGraphicsApi* m_graphicsApi;

	RingBuffer<ConstBufferPage> m_largePages;
	RingBuffer<ConstBufferPage> m_pages;
	std::mutex m_mutex;

	uint64_t m_currFrameID = 1;
	uint64_t m_lastFinishedFrameID = 0;

protected:
	// From ( https://msdn.microsoft.com/en-us/library/windows/desktop/dn899216%28v=vs.85%29.aspx )
	// "Linear subresource copying must be aligned to 512 bytes"
	static constexpr size_t ALIGNEMENT = 512;
	static constexpr size_t PAGE_SIZE = 64*1024;

	static constexpr size_t MAX_PERMANENT_LARGE_PAGE_COUNT = 5;

	static size_t SnapUpward(size_t value, size_t gridSize);
protected:
	ConstBufferPage CreatePage();
	ConstBufferPage CreateLargePage(size_t fittingSize);
	bool HasBecomeAvailable(const ConstBufferPage& page);
	void MarkEmptyIfRecycled(ConstBufferPage& page);
};

} // namespace gxeng
} // namespace inl
