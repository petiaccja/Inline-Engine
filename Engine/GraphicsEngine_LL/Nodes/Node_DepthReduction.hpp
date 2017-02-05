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


class DepthReduction :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<gxeng::RWTextureView2D>,
	public WindowResizeListener
{
public:
	DepthReduction(gxapi::IGraphicsApi* graphicsApi, unsigned width, unsigned height);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

	void WindowResized(unsigned width, unsigned height) override;

protected:
	unsigned m_width;
	unsigned m_height;

	gxeng::RWTextureView2D m_uav;

protected:
	GraphicsContext m_graphicsContext;
	Binder m_binder;
	BindParameter m_depthBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_CSO;

private:
	void InitRenderTarget();
	void RenderScene(
		const gxeng::RWTextureView2D& uav,
		pipeline::Texture2D& depthTex,
		ComputeCommandList& commandList);
};


} // namespace inl::gxeng::nodes

