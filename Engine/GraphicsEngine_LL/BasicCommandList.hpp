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
	BasicCommandList(gxapi::IGraphicsApi* gxApi, CommandAllocatorPool& cmdAllocatorPool, inl::gxapi::eCommandListType type);
	
protected:
	void UseResource(GenericBuffer* resource);
	
private:
	std::vector<GenericBuffer*> m_usedResources;
	gxapi::ICommandAllocator* m_cmdAllocator;
};



} // namespace gxeng
} // namespace inl