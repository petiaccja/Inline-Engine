#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"
#include "CommandAllocatorPool.hpp"

#include <vector>


namespace inl {
namespace gxeng {



class BasicCommandList {
public:
	struct Decomposition {
		CommandAllocatorPool* commandAllocatorPool;
		gxapi::ICommandAllocator* commandAllocator;
		gxapi::ICommandList* commandList;
		std::vector<GenericBuffer*> usedResources;
	};
private:
	struct AllocDeleter {
		AllocDeleter(CommandAllocatorPool& pool) : pool(&pool) {}
		CommandAllocatorPool* pool;
		void operator()(gxapi::ICommandAllocator* arg) { pool->RecycleAllocator(arg); }
	};
public:
	BasicCommandList(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList(BasicCommandList&& rhs);
	BasicCommandList& operator=(const BasicCommandList& rhs) = delete; // could be, but big perf hit, better not allow user
	BasicCommandList& operator=(BasicCommandList&& rhs);
	virtual ~BasicCommandList() = default;

	gxapi::eCommandListType GetType() const { return m_commandList->GetType(); }

	virtual Decomposition Decompose();
protected:
	BasicCommandList(CommandAllocatorPool& cmdAllocatorPool, inl::gxapi::eCommandListType type);
	void UseResource(GenericBuffer* resource);
	gxapi::ICommandList* GetCommandList() { return m_commandList.get(); }

private:
	std::vector<GenericBuffer*> m_usedResources;
	std::unique_ptr<gxapi::ICommandAllocator, AllocDeleter> m_commandAllocator;
	std::unique_ptr<gxapi::ICommandList> m_commandList;
	CommandAllocatorPool* m_commandAllocatorPool;
};



} // namespace gxeng
} // namespace inl