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


class CSM :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<const EntityCollection<MeshEntity>*, pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>
{
public:
	CSM(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

protected:
	DepthStencilView2D m_dsv;
	TextureView2D m_depthTargetSrv;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_uniformsBindParam;
	BindParameter m_lightMVPBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitRenderTarget(unsigned width, unsigned height);
	void RenderScene(
		DepthStencilView2D& dsv,
		const EntityCollection<MeshEntity>& entities,
		pipeline::Texture2D& lightMVPTex,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes

