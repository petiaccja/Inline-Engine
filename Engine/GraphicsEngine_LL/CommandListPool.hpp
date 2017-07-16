#pragma once

#include <BaseLibrary/Memory/SlabAllocatorEngine.hpp>
#include <GraphicsApi_LL/ICommandList.hpp>
#include <GraphicsApi_LL/IGraphicsApi.hpp>

#include <vector>
#include <mutex>
#include <algorithm>
#include <map>
#include <cassert>

#include <BaseLibrary/Logging/LogStream.hpp>


namespace inl::gxeng {


namespace impl {

class CommandListPoolBase {
public:
	struct Deleter {
	public:
		Deleter() : m_container(nullptr) {}
		Deleter(const Deleter&) = default;
		Deleter(Deleter&&) = default;
		Deleter& operator=(const Deleter&) = default;
		Deleter& operator=(Deleter&&) = default;
		explicit Deleter(CommandListPoolBase* container) : m_container(container) {}
		void operator()(gxapi::ICommandList* object) const {
			assert(m_container != nullptr);
			m_container->RecycleList(object);
		}
	private:
		CommandListPoolBase* m_container;
	};
	using UniquePtr = std::unique_ptr<gxapi::ICommandList, Deleter>;
public:
	virtual ~CommandListPoolBase() {}
	virtual UniquePtr RequestList() = 0;
	virtual void RecycleList(gxapi::ICommandList*) = 0;
};


template <gxapi::eCommandListType TYPE>
class CommandListPool : public CommandListPoolBase {
public:
public:
	explicit CommandListPool(gxapi::IGraphicsApi* gxApi, size_t initialSize = 1);
	CommandListPool(const CommandListPool&) = delete;
	CommandListPool(CommandListPool&& rhs);

	CommandListPool& operator=(const CommandListPool&) = delete;
	CommandListPool& operator=(CommandListPool&& rhs);


	UniquePtr RequestList() override;
	void RecycleList(gxapi::ICommandAllocator* allocator) override;
	void Reset(size_t initialSize = 1);

	gxapi::IGraphicsApi* GetGraphicsApi() const { return m_gxApi; }

	void SetLogStream(LogStream* logStream) { m_logStream = logStream; }
	LogStream* GetLogStream() const { return m_logStream; }
private:
	std::vector<std::unique_ptr<gxapi::ICommandAllocator>> m_pool;
	SlabAllocatorEngine m_allocator;
	gxapi::IGraphicsApi* m_gxApi;
	std::map<gxapi::ICommandAllocator*, size_t> m_addressToIndex;
	LogStream* m_logStream = nullptr;

	std::mutex m_mtx;
};



template <gxapi::eCommandListType TYPE>
CommandAllocatorPool<TYPE>::CommandAllocatorPool(gxapi::IGraphicsApi* gxApi, size_t initialSize)
	: m_pool(initialSize), m_allocator(initialSize), m_gxApi(gxApi)
{}


template <gxapi::eCommandListType TYPE>
CommandAllocatorPool<TYPE>::CommandAllocatorPool(CommandAllocatorPool&& rhs)
	: m_pool(std::move(rhs.m_pool)), m_allocator(std::move(rhs.m_allocator)), m_gxApi(rhs.m_gxApi)
{}


template <gxapi::eCommandListType TYPE>
CommandAllocatorPool<TYPE>& CommandAllocatorPool<TYPE>::operator=(CommandAllocatorPool&& rhs) {
	m_pool = std::move(rhs.m_pool);
	m_allocator = std::move(rhs.m_allocator);
	m_gxApi = rhs.m_gxApi;

	return *this;
}


template <gxapi::eCommandListType TYPE>
auto CommandAllocatorPool<TYPE>::RequestAllocator() -> UniquePtr {
	std::lock_guard<std::mutex> lkg(m_mtx);

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
		return UniquePtr{ m_pool[index].get(), Deleter{ this } };
	}
	else {
		std::unique_ptr<gxapi::ICommandAllocator> ptr(m_gxApi->CreateCommandAllocator(TYPE));
		m_addressToIndex[ptr.get()] = index;
		m_pool[index] = std::move(ptr);
		return UniquePtr{ m_pool[index].get(), Deleter{ this } };
	}
}


template <gxapi::eCommandListType TYPE>
void CommandAllocatorPool<TYPE>::RecycleAllocator(gxapi::ICommandAllocator* allocator) {
	std::lock_guard<std::mutex> lkg(m_mtx);

	allocator->Reset();
	assert(m_addressToIndex.count(allocator) > 0);
	size_t index = m_addressToIndex[allocator];
	m_allocator.Deallocate(index);
}


template <gxapi::eCommandListType TYPE>
void CommandAllocatorPool<TYPE>::Reset(size_t initialSize) {
	m_pool.clear();
	m_allocator.Reset();
	m_pool.resize(initialSize);
	m_allocator.Resize(initialSize);
}



} // namespace impl








} // namespace inl::gxeng
