#pragma once

#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "HighLevelDescHeap.hpp"
#include <vector>
#include <map>
#include <mutex>


namespace inl {
namespace gxeng {



class ScratchSpacePool {
public:
	struct Deleter {
	public:
		Deleter() : m_container(nullptr) {}
		explicit Deleter(ScratchSpacePool* container) : m_container(container) {}
		void operator()(ScratchSpace* object) const {
			m_container->RecycleScratchSpace(object);
		}
	private:
		ScratchSpacePool* m_container;
	};

	using UniquePtr = std::unique_ptr<ScratchSpace, Deleter>;
public:
	ScratchSpacePool(gxapi::IGraphicsApi* gxApi, gxapi::eDescriptorHeapType type);
	ScratchSpacePool(const ScratchSpacePool&) = delete;
	ScratchSpacePool(ScratchSpacePool&&) = default;
	ScratchSpacePool& operator=(const ScratchSpacePool&) = delete;
	ScratchSpacePool& operator=(ScratchSpacePool&& rhs) = default;;

	UniquePtr RequestScratchSpace();
	void RecycleScratchSpace(ScratchSpace* scratchSpace);
private:
	std::vector<std::unique_ptr<ScratchSpace>> m_pool;
	gxapi::eDescriptorHeapType m_type;
	exc::SlabAllocatorEngine m_allocator;
	gxapi::IGraphicsApi* m_gxApi;
	std::map<ScratchSpace*, size_t> m_addressToIndex;

	std::mutex m_mutex;
};


using ScratchSpacePtr = ScratchSpacePool::UniquePtr;



} // namespace gxeng
} // namespace inl
