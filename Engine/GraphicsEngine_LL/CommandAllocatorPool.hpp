#pragma once

#include "../BaseLibrary/Memory/SlabAllocatorEngine.hpp"
#include "../GraphicsApi_LL/ICommandAllocator.hpp"
#include "../GraphicsApi_LL/IGraphicsApi.hpp"

#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <cassert>


namespace inl {
namespace gxeng {


namespace impl {

	template <inl::gxapi::eCommandListType TYPE>
	class CommandAllocatorPool {
	public:
		CommandAllocatorPool(inl::gxapi::IGraphicsApi* gxApi, size_t initialSize = 1);
		CommandAllocatorPool(const CommandAllocatorPool&) = delete;
		CommandAllocatorPool(CommandAllocatorPool&& rhs);

		CommandAllocatorPool& operator=(const CommandAllocatorPool&) = delete;
		CommandAllocatorPool& operator=(CommandAllocatorPool&& rhs);


		inl::gxapi::ICommandAllocator* RequestAllocator();
		void RecycleAllocator(inl::gxapi::ICommandAllocator* allocator);
		void Reset(size_t initialSize = 1);
	private:
		std::vector<std::unique_ptr<inl::gxapi::ICommandAllocator>> m_pool;
		exc::SlabAllocatorEngine m_allocator;
		inl::gxapi::IGraphicsApi* m_gxApi;
		std::map<inl::gxapi::ICommandAllocator*, size_t> addressToIndex;
	};



	template <inl::gxapi::eCommandListType TYPE>
	CommandAllocatorPool<TYPE>::CommandAllocatorPool(inl::gxapi::IGraphicsApi* gxApi, size_t initialSize)
		: m_pool(initialSize), m_allocator(initialSize), m_gxApi(gxApi)
	{}


	template <inl::gxapi::eCommandListType TYPE>
	CommandAllocatorPool<TYPE>::CommandAllocatorPool(CommandAllocatorPool&& rhs)
		: m_pool(std::move(rhs.m_pool)), m_allocator(std::move(rhs.m_allocator)), m_gxApi(rhs.m_gxApi)
	{}


	template <inl::gxapi::eCommandListType TYPE>
	CommandAllocatorPool<TYPE>& CommandAllocatorPool<TYPE>::operator=(CommandAllocatorPool&& rhs) {
		m_pool = std::move(rhs.m_pool);
		m_allocator = std::move(rhs.m_allocator);
		m_gxApi = rhs.m_gxApi;

		return *this;
	}


	template <inl::gxapi::eCommandListType TYPE>
	inl::gxapi::ICommandAllocator* CommandAllocatorPool<TYPE>::RequestAllocator() {
		size_t index;
		try {
			index = m_allocator.Allocate();
		}
		catch (std::bad_alloc&) {
			size_t currentSize = m_pool.size();
			size_t newSize = std::max(currentSize + 1, size_t(currentSize * 1.25));
			m_pool.resize(newSize);
			m_allocator.Resize(newSize);

			index = m_allocator.Allocate();
		}

		if (m_pool[index] != nullptr) {
			return m_pool[index].get();
		}
		else {
			auto* cmdAlloc = m_gxApi->CreateCommandAllocator(TYPE);
			addressToIndex[cmdAlloc] = index;
			m_pool[index].reset(cmdAlloc);
		}
	}


	template <inl::gxapi::eCommandListType TYPE>
	void CommandAllocatorPool<TYPE>::RecycleAllocator(inl::gxapi::ICommandAllocator* allocator) {
		assert(addressToIndex.count(allocator) > 0);
		size_t index = addressToIndex[allocator];
		m_allocator.Deallocate(index);
	}


	template <inl::gxapi::eCommandListType TYPE>
	void CommandAllocatorPool<TYPE>::Reset(size_t initialSize) {
		m_pool.clear();
		m_allocator.Reset();
		m_pool.resize(initialSize);
		m_allocator.Resize(initialSize);
	}

} // namespace impl



class CommandAllocatorPool {
public:
	CommandAllocatorPool(gxapi::IGraphicsApi* gxApi);
	CommandAllocatorPool(const CommandAllocatorPool&) = delete;
	CommandAllocatorPool(CommandAllocatorPool&& rhs) = default;
	CommandAllocatorPool& operator=(const CommandAllocatorPool&) = delete;
	CommandAllocatorPool& operator=(CommandAllocatorPool&& rhs) = default;

	gxapi::ICommandAllocator* RequestAllocator(gxapi::eCommandListType type);
	void RecycleAllocator(gxapi::ICommandAllocator* allocator);
private:
	impl::CommandAllocatorPool<gxapi::eCommandListType::GRAPHICS> m_gxPool;
	impl::CommandAllocatorPool<gxapi::eCommandListType::COMPUTE> m_cuPool;
	impl::CommandAllocatorPool<gxapi::eCommandListType::COPY> m_cpPool;
};



} // namespace gxeng
} // namespace inl