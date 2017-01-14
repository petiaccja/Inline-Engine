#pragma once

#include "Node_GenCSM.hpp"

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../TextureViewPack.hpp"
#include "../WindowResizeListener.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class CombineGBuffer :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<DepthStencilPack, RenderTargetPack, RenderTargetPack, const ShadowCascades*, const Camera*, const DirectionalLight*>,
	virtual public exc::OutputPortConfig<RenderTargetPack>,
	public WindowResizeListener
{
public:
	CombineGBuffer(gxapi::IGraphicsApi* graphicsApi, unsigned width, unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

	void WindowResized(unsigned width, unsigned height) override;

protected:
	unsigned m_width;
	unsigned m_height;
	RenderTargetPack m_renderTarget;
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_sunBindParam;
	BindParameter m_transformBindParam;
	BindParameter m_albedoRoughnessBindParam;
	BindParameter m_normalBindParam;
	BindParameter m_depthBindParam;
	BindParameter m_shadowMapBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitBuffer();
	void RenderCombined(
		Texture2DSRV& depthStencil,
		Texture2DSRV& albedoRoughness,
		Texture2DSRV& normal,
		const ShadowCascades* sunShadowMaps,
		const Camera* camera,
		const DirectionalLight* sun,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
