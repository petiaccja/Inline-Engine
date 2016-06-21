#pragma once

#include "../GraphicsApi_LL/IGraphicsApi.hpp"
#include "../GraphicsApi_LL/Common.hpp"

#include "GpuBuffer.hpp"
#include "CommandAllocatorPool.hpp"

#include <vector>


namespace inl {
namespace gxeng {



class BasicCommandList {
	struct AllocDeleter {
		AllocDeleter(CommandAllocatorPool& pool) : pool(pool) {}
		CommandAllocatorPool& pool;
		void operator()(gxapi::ICommandAllocator* arg) { pool.RecycleAllocator(arg); }
	};
public:
	gxapi::eCommandListType GetType() const { return m_commandList->GetType(); }
	virtual ~BasicCommandList();
protected:
	BasicCommandList(CommandAllocatorPool& cmdAllocatorPool, inl::gxapi::eCommandListType type);

	void UseResource(GenericResource* resource);

protected:
	std::vector<GenericResource*> m_usedResources;
	std::unique_ptr<gxapi::ICommandAllocator, AllocDeleter> m_commandAllocator;
	std::unique_ptr<gxapi::ICommandList> m_commandList;
};



} // namespace gxeng
} // namespace inl