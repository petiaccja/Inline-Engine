#include "CommandAllocatorPool.hpp"

#include <algorithm>



namespace inl {
namespace gxeng {


CommandAllocatorPool::CommandAllocatorPool(gxapi::IGraphicsApi* gxApi) 
	: m_gxPool(gxApi), m_cuPool(gxApi), m_cpPool(gxApi)
{}


gxapi::ICommandAllocator* CommandAllocatorPool::RequestAllocator(gxapi::eCommandListType type) {
	switch (type)
	{
		case gxapi::eCommandListType::COPY:
			return m_cpPool.RequestAllocator();
		case gxapi::eCommandListType::COMPUTE:
			return m_cuPool.RequestAllocator();
		case gxapi::eCommandListType::GRAPHICS:
			return m_gxPool.RequestAllocator();
		case gxapi::eCommandListType::BUNDLE:
			return nullptr;
		default:
			return nullptr;
	}
}


void CommandAllocatorPool::RecycleAllocator(gxapi::ICommandAllocator* allocator) {
	switch (allocator->GetType())
	{
		case gxapi::eCommandListType::COPY:
			m_cpPool.RecycleAllocator(allocator);
		case gxapi::eCommandListType::COMPUTE:
			m_cuPool.RecycleAllocator(allocator);
		case gxapi::eCommandListType::GRAPHICS:
			m_gxPool.RecycleAllocator(allocator);
	}
}

gxapi::IGraphicsApi* CommandAllocatorPool::GetGraphicsApi() const {
	return m_gxPool.GetGraphicsApi();
}



} // namespace gxeng
} // namespace inl