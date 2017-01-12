#include "ExecutionContext.hpp"

#include "Scene.hpp"
#include "Camera.hpp"


namespace inl {
namespace gxeng {



GraphicsCommandList ExecutionContext::GetGraphicsCommandList() const {
	return GraphicsCommandList(
		m_frameContext->gxApi,
		*m_frameContext->commandAllocatorPool,
		*m_frameContext->scratchSpacePool
	);
}


ComputeCommandList ExecutionContext::GetComputeCommandList() const {
	return ComputeCommandList(
		m_frameContext->gxApi,
		*m_frameContext->commandAllocatorPool,
		*m_frameContext->scratchSpacePool
	);
}


CopyCommandList ExecutionContext::GetCopyCommandList() const {
	return CopyCommandList(
		m_frameContext->gxApi,
		*m_frameContext->commandAllocatorPool,
		*m_frameContext->scratchSpacePool
	);
}


std::chrono::nanoseconds ExecutionContext::GetFrameTime() const {
	return m_frameContext->frameTime;
}

std::chrono::nanoseconds ExecutionContext::GetAbsoluteTime() const {
	return m_frameContext->absoluteTime;
}


//------------------------------------------------------------------------------
// Special access functions
//------------------------------------------------------------------------------

const Scene* ExecutionContext::GetSceneByName(const std::string& name) const {
	for (auto scene : *m_frameContext->scenes) {
		if (scene->GetName() == name) {
			return scene;
		}
	}
	return nullptr;
}


const Camera* ExecutionContext::GetCameraByName(const std::string& name) const {
	for (auto camera : *m_frameContext->cameras) {
		if (camera->GetName() == name) {
			return camera;
		}
	}
	return nullptr;
}

RenderTargetView2D* ExecutionContext::GetBackBuffer() const {
	return m_frameContext->backBuffer;
}



} // namespace gxeng
} // namespace inl