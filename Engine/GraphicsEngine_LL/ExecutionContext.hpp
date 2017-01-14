#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"
#include "VolatileViewHeap.hpp"

#include "FrameContext.hpp"


#include <chrono>


namespace inl {
namespace gxeng {


//------------------------------------------------------------------------------
// Prototypes of classes used.
//------------------------------------------------------------------------------
class Scene;

namespace nodes {
class GetBackBuffer;
class GetSceneByName;
class GetCameraByName;
}



//------------------------------------------------------------------------------
// Special access contexts.
//------------------------------------------------------------------------------
class SceneAccessContext {
	friend class nodes::GetSceneByName;
	friend class nodes::GetCameraByName;
public:
	virtual ~SceneAccessContext() {}
protected:
	virtual const Scene* GetSceneByName(const std::string& name) const = 0;
	virtual const Camera* GetCameraByName(const std::string& name) const = 0;
};



class SwapChainAccessContext {
	friend class nodes::GetBackBuffer;
public:
	virtual ~SwapChainAccessContext() {}
protected:
	virtual RenderTargetView2D* GetBackBuffer() const = 0;
};



//------------------------------------------------------------------------------
// General execution context.
//------------------------------------------------------------------------------
class ExecutionContext 
	: public SceneAccessContext,
	public SwapChainAccessContext
{
public:
	ExecutionContext(FrameContext* frameContext) : m_frameContext(frameContext) {
		assert(frameContext != nullptr);
	}

	GraphicsCommandList GetGraphicsCommandList() const;
	ComputeCommandList GetComputeCommandList() const;
	CopyCommandList GetCopyCommandList() const;

	//TODO GetVolatileViewHeap() const;

	std::chrono::nanoseconds GetFrameTime() const;
	std::chrono::nanoseconds GetAbsoluteTime() const;

	uint64_t GetFrameNumber() const;

protected:
	const Scene* GetSceneByName(const std::string& name) const override;
	const Camera* GetCameraByName(const std::string& name) const override;
	RenderTargetView2D* GetBackBuffer() const override;	
private:
	FrameContext* m_frameContext;
};



} // namespace gxeng
} // namespace inl