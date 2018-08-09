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
		Deleter() noexcept : m_container(nullptr) {}
		Deleter(const Deleter& rhs) noexcept : m_container(rhs.m_container) {}
		Deleter(Deleter&& rhs) noexcept : m_container(rhs.m_container) { rhs.m_container = nullptr; }
		Deleter& operator=(const Deleter& rhs) noexcept {
			m_container = rhs.m_container;
			return *this;
		}
		Deleter& operator=(Deleter&& rhs) noexcept { 
			m_container = rhs.m_container; rhs.m_container = nullptr;
			return *this;
		}
		explicit Deleter(CommandListPoolBase* container) noexcept : m_container(container) {}
		~Deleter() noexcept = default;
		void operator()(gxapi::ICommandList* object) const {
			assert(m_container != nullptr);
			m_container->RecycleList(object);
		}
	private:
		CommandListPoolBase* m_container;
	};
	using UniquePtr = std::unique_ptr<gxapi::ICommandList, Deleter>;
	using GraphicsUniquePtr = std::unique_ptr<gxapi::IGraphicsCommandList, Deleter>;
	using ComputeUniquePtr = std::unique_ptr<gxapi::IComputeCommandList, Deleter>;
	using CopyUniquePtr = std::unique_ptr<gxapi::ICopyCommandList, Deleter>;
public:
	virtual ~CommandListPoolBase() {}
	virtual UniquePtr RequestList(gxapi::ICommandAllocator*) = 0;
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


	UniquePtr RequestList(gxapi::ICommandAllocator* allocator) override;
	void RecycleList(gxapi::ICommandList* list) override;
	void Reset(size_t initialSize = 1);

	gxapi::IGraphicsApi* GetGraphicsApi() const { return m_gxApi; }

	void SetLogStream(LogStream* logStream) { m_logStream = logStream; }
	LogStream* GetLogStream() const { return m_logStream; }
private:
	std::vector<std::unique_ptr<gxapi::ICommandList>> m_pool;
	SlabAllocatorEngine m_allocator;
	gxapi::IGraphicsApi* m_gxApi;
	std::map<gxapi::ICommandList*, size_t> m_addressToIndex;
	LogStream* m_logStream = nullptr;

	std::mutex m_mtx;
};



template <gxapi::eCommandListType TYPE>
CommandListPool<TYPE>::CommandListPool(gxapi::IGraphicsApi* gxApi, size_t initialSize)
	: m_pool(initialSize), m_allocator(initialSize), m_gxApi(gxApi)
{}


template <gxapi::eCommandListType TYPE>
CommandListPool<TYPE>::CommandListPool(CommandListPool&& rhs)
	: m_pool(std::move(rhs.m_pool)), m_allocator(std::move(rhs.m_allocator)), m_gxApi(rhs.m_gxApi)
{}


template <gxapi::eCommandListType TYPE>
CommandListPool<TYPE>& CommandListPool<TYPE>::operator=(CommandListPool&& rhs) {
	m_pool = std::move(rhs.m_pool);
	m_allocator = std::move(rhs.m_allocator);
	m_gxApi = rhs.m_gxApi;

	return *this;
}


template <gxapi::eCommandListType TYPE>
auto CommandListPool<TYPE>::RequestList(gxapi::ICommandAllocator* allocator) -> UniquePtr {
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
		dynamic_cast<gxapi::ICopyCommandList*>(m_pool[index].get())->Reset(allocator, nullptr);
		return UniquePtr{ m_pool[index].get(), Deleter{ this } };
	}
	else {
		gxapi::CommandListDesc desc;
		desc.allocator = allocator;
		desc.initialState = nullptr;
		std::unique_ptr<gxapi::ICommandList> ptr(m_gxApi->CreateCommandList(TYPE, desc));
		m_addressToIndex[ptr.get()] = index;
		m_pool[index] = std::move(ptr);
	
		return UniquePtr{ m_pool[index].get(), Deleter{ this } };
	}
}


template <gxapi::eCommandListType TYPE>
void CommandListPool<TYPE>::RecycleList(gxapi::ICommandList* list) {
	std::lock_guard<std::mutex> lkg(m_mtx);

	assert(m_addressToIndex.count(list) > 0);
	size_t index = m_addressToIndex[list];
	m_allocator.Deallocate(index);
}


template <gxapi::eCommandListType TYPE>
void CommandListPool<TYPE>::Reset(size_t initialSize) {
	m_pool.clear();
	m_allocator.Reset();
	m_pool.resize(initialSize);
	m_allocator.Resize(initialSize);
}



} // namespace impl


using CmdListPtr = impl::CommandListPoolBase::UniquePtr;
using GraphicsCmdListPtr = impl::CommandListPoolBase::GraphicsUniquePtr;
using ComputeCmdListPtr = impl::CommandListPoolBase::ComputeUniquePtr;
using CopyCmdListPtr = impl::CommandListPoolBase::CopyUniquePtr;

class CommandListPool {
public:
public:
	explicit CommandListPool(gxapi::IGraphicsApi* gxApi);
	CommandListPool(const CommandListPool&) = delete;
	CommandListPool(CommandListPool&& rhs) = default;
	CommandListPool& operator=(const CommandListPool&) = delete;
	CommandListPool& operator=(CommandListPool&& rhs) = default;

	CmdListPtr RequestList(gxapi::eCommandListType type, gxapi::ICommandAllocator* allocator);
	GraphicsCmdListPtr RequestGraphicsList(gxapi::ICommandAllocator* allocator);
	ComputeCmdListPtr RequestComputeList(gxapi::ICommandAllocator* allocator);
	CopyCmdListPtr RequestCopyList(gxapi::ICommandAllocator* allocator);
	void RecycleList(gxapi::ICommandList* list);
	void Clear();

	gxapi::IGraphicsApi* GetGraphicsApi() const;

	void SetLogStream(LogStream* logStream);
	LogStream* GetLogStream() const;
private:
	impl::CommandListPool<gxapi::eCommandListType::GRAPHICS> m_gxPool;
	impl::CommandListPool<gxapi::eCommandListType::COMPUTE> m_cuPool;
	impl::CommandListPool<gxapi::eCommandListType::COPY> m_cpPool;
};





} // namespace inl::gxeng
