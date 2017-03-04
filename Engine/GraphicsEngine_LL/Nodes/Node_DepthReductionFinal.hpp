#pragma once

#include "../GraphicsNode.hpp"

#include "Node_GenCSM.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../Mesh.hpp"
#include "../ConstBufferHeap.hpp"
#include "../GraphicsContext.hpp"
#include "../PipelineTypes.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"

namespace inl::gxeng::nodes {


class DepthReductionFinal :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D, pipeline::Texture2D, const Camera*>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D, pipeline::Texture2D, pipeline::Texture2D, pipeline::Texture2D>
{
public:
	DepthReductionFinal(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

protected:
	gxeng::RWTextureView2D m_light_mvp_uav;
	gxeng::TextureView2D m_light_mvp_srv;

	gxeng::RWTextureView2D m_shadow_mx_uav;
	gxeng::TextureView2D m_shadow_mx_srv;

	gxeng::RWTextureView2D m_csm_splits_uav;
	gxeng::TextureView2D m_csm_splits_srv;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_reductionBindParam;
	BindParameter m_outputBindParam0;
	BindParameter m_outputBindParam1;
	BindParameter m_outputBindParam2;
	BindParameter m_uniformsBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

private:
	void InitRenderTarget();
	void RenderScene(
		const gxeng::RWTextureView2D& light_mvp_uav,
		const gxeng::RWTextureView2D& shadow_mx_uav,
		const gxeng::RWTextureView2D& csm_splits_uav,
		pipeline::Texture2D& reductionTex,
		const Camera* camera,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes

