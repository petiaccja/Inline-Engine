#pragma once

#include "../GraphicsNode.hpp"

#include "Node_GenCSM.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../PipelineTypes.hpp"
#include "../WindowResizeListener.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {

class ForwardRender :
	virtual public GraphicsNode,
	// Inputs: depth stencil (from depth prepass), geometry, camera, sun
	virtual public exc::InputPortConfig<pipeline::Texture2D, const EntityCollection<MeshEntity>*, const Camera*, const DirectionalLight*>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>,
	public WindowResizeListener
{
public:
	ForwardRender(gxapi::IGraphicsApi* graphicsApi, unsigned width, unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

	void WindowResized(unsigned width, unsigned height) override;

protected:
	unsigned m_width;
	unsigned m_height;

	RenderTargetView2D m_rtv;
	TextureView2D m_renderTargetSrv;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_transformBindParam;
	BindParameter m_sunBindParam;
	BindParameter m_albedoBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitRenderTarget();
	void RenderScene(
		DepthStencilView2D& dsv,
		const EntityCollection<MeshEntity>& entities,
		const Camera* camera,
		const DirectionalLight* sun,
		GraphicsCommandList& commandList);
};

} // namespace inl::gxeng::nodes

