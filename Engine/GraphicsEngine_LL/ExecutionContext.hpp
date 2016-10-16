#pragma once

#include "GraphicsCommandList.hpp"
#include "ComputeCommandList.hpp"
#include "CopyCommandList.hpp"

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
}



//------------------------------------------------------------------------------
// Special access contexts.
//------------------------------------------------------------------------------
class SceneAccessContext {
	friend class nodes::GetSceneByName;
public:
	virtual ~SceneAccessContext() {}
protected:
	virtual const Scene* GetSceneByName(const std::string& name) const = 0;
};



class SwapChainAccessContext {
	friend class nodes::GetBackBuffer;
public:
	virtual ~SwapChainAccessContext() {}
protected:
	virtual BackBuffer* GetBackBuffer() const = 0;
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

	std::chrono::nanoseconds GetFrameTime() const;
	std::chrono::nanoseconds GetAbsoluteTime() const;

protected:
	const Scene* GetSceneByName(const std::string& name) const override;
	BackBuffer* GetBackBuffer() const override;	
private:
	FrameContext* m_frameContext;
};



} // namespace gxeng
} // namespace inl