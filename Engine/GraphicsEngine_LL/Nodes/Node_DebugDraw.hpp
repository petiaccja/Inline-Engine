#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

#include <optional>

namespace inl::gxeng::nodes {

/// <summary>
/// Inputs: render target, scene objects, light cascade MVP transform matrices in a texture
/// Output: render target
/// </summary>
class DebugDraw :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<Texture2D, const BasicCamera*>,
	virtual public exc::OutputPortConfig<Texture2D>
{
public:
	DebugDraw();

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override;
	void Reset() override;
	void Setup(SetupContext& context) override;
	void Execute(RenderContext& context) override;

protected:
	std::optional<Binder> m_binder;
	BindParameter m_uniformsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_LinePSO;
	std::unique_ptr<gxapi::IPipelineState> m_TrianglePSO;

private: // render context
	RenderTargetView2D m_target;
};


} // namespace inl::gxeng::nodes

