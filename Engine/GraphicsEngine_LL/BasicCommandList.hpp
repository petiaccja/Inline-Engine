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
	BasicCommandList(CommandAllocatorPool& cmdAllocatorPool, inl::gxapi::eCommandListType type);
	~BasicCommandList();

protected:
	void UseResource(GenericBuffer* resource);
	
private:
	std::vector<GenericBuffer*> m_usedResources;

	std::unique_ptr<gxapi::ICommandAllocator, AllocDeleter> m_commandAllocator;
	std::unique_ptr<gxapi::ICommandList> m_commandList;
};



} // namespace gxeng
} // namespace inl