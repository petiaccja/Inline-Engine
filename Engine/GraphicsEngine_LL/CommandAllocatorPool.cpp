#include "CommandAllocatorPool.hpp"

#include <algorithm>
#include <cassert>



namespace inl {
namespace gxeng {


CommandAllocatorPool::CommandAllocatorPool(gxapi::IGraphicsApi* gxApi)
	: m_gxPool(gxApi), m_cuPool(gxApi), m_cpPool(gxApi)
{}


auto CommandAllocatorPool::RequestAllocator(gxapi::eCommandListType type) -> CmdAllocPtr {
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
		default:
			assert(false); // hülye vagy bazmeg
	}
}


void CommandAllocatorPool::Clear() {
	m_cpPool.Reset();
	m_cuPool.Reset();
	m_gxPool.Reset();
}


gxapi::IGraphicsApi* CommandAllocatorPool::GetGraphicsApi() const {
	return m_gxPool.GetGraphicsApi();
}



void CommandAllocatorPool::SetLogStream(LogStream* logStream){
	m_cpPool.SetLogStream(logStream);
	m_cuPool.SetLogStream(logStream);
	m_gxPool.SetLogStream(logStream);
}

LogStream* CommandAllocatorPool::GetLogStream() const {
	return m_gxPool.GetLogStream();
}



} // namespace gxeng
} // namespace inl