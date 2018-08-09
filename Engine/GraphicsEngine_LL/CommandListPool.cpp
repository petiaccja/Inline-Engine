#include "CommandListPool.hpp"

#include <algorithm>
#include <cassert>

#include <BaseLibrary/SmartPtrCast.hpp>



namespace inl::gxeng {


CommandListPool::CommandListPool(gxapi::IGraphicsApi* gxApi)
	: m_gxPool(gxApi), m_cuPool(gxApi), m_cpPool(gxApi)
{}


auto CommandListPool::RequestList(gxapi::eCommandListType type, gxapi::ICommandAllocator* allocator) -> CmdListPtr {
	switch (type)
	{
	case gxapi::eCommandListType::COPY:
		return m_cpPool.RequestList(allocator);
	case gxapi::eCommandListType::COMPUTE:
		return m_cuPool.RequestList(allocator);
	case gxapi::eCommandListType::GRAPHICS:
		return m_gxPool.RequestList(allocator);
	case gxapi::eCommandListType::BUNDLE:
		return nullptr;
	default:
		return nullptr;
	}
}

GraphicsCmdListPtr CommandListPool::RequestGraphicsList(gxapi::ICommandAllocator* allocator) {
	return dynamic_pointer_cast<gxapi::IGraphicsCommandList>(RequestList(gxapi::eCommandListType::GRAPHICS, allocator));
}
ComputeCmdListPtr CommandListPool::RequestComputeList(gxapi::ICommandAllocator* allocator) {
	return dynamic_pointer_cast<gxapi::IComputeCommandList>(RequestList(gxapi::eCommandListType::COMPUTE, allocator));
}
CopyCmdListPtr CommandListPool::RequestCopyList(gxapi::ICommandAllocator* allocator) {
	return dynamic_pointer_cast<gxapi::ICopyCommandList>(RequestList(gxapi::eCommandListType::COPY, allocator));
}


void CommandListPool::RecycleList(gxapi::ICommandList* list) {
	if (auto x = dynamic_cast<gxapi::IComputeCommandList*>(list)) {
		x->ResetState(nullptr);
	}
	switch (list->GetType())
	{
	case gxapi::eCommandListType::COPY:
		m_cpPool.RecycleList(list);
	case gxapi::eCommandListType::COMPUTE:
		m_cuPool.RecycleList(list);
	case gxapi::eCommandListType::GRAPHICS:
		m_gxPool.RecycleList(list);
	default:
		assert(false); // hülye vagy bazmeg
	}
}


void CommandListPool::Clear() {
	m_cpPool.Reset();
	m_cuPool.Reset();
	m_gxPool.Reset();
}


gxapi::IGraphicsApi* CommandListPool::GetGraphicsApi() const {
	return m_gxPool.GetGraphicsApi();
}



void CommandListPool::SetLogStream(LogStream* logStream) {
	m_cpPool.SetLogStream(logStream);
	m_cuPool.SetLogStream(logStream);
	m_gxPool.SetLogStream(logStream);
}

LogStream* CommandListPool::GetLogStream() const {
	return m_gxPool.GetLogStream();
}



} // namespace inl::gxeng