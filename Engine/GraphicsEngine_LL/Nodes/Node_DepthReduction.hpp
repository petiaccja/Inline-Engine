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


class DepthReduction :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D, pipeline::Texture2D>
{
public:
	DepthReduction(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

protected:
	unsigned m_width;
	unsigned m_height;

	gxeng::RWTextureView2D m_uav;
	gxeng::TextureView2D m_srv;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_depthBindParam;
	BindParameter m_outputBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

private:
	void InitRenderTarget();
	void RenderScene(
		const gxeng::RWTextureView2D& uav,
		pipeline::Texture2D& depthTex,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes

