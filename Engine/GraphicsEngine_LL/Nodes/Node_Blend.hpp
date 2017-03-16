#pragma once


#include "../GraphicsNode.hpp"

#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../GraphicsContext.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class Blend :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D, pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<pipeline::Texture2D>
{
public:
	enum BlendMode { CASUAL_ALPHA_BLEND };

public:
	Blend(gxapi::IGraphicsApi* graphicsApi, BlendMode mode);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override;

protected:
	static constexpr auto COLOR_FORMAT = gxapi::eFormat::R8G8B8A8_UNORM;

protected:
	RenderTargetView2D m_rtv;
	TextureView2D m_renderTargetSrv;

	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

	GraphicsContext m_graphicsContext;

protected:
	BlendMode m_mode;

	Binder m_binder;
	BindParameter m_tex0Param;
	BindParameter m_tex1Param;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void InitRenderTarget(unsigned width, unsigned height);
	void Render(
		const TextureView2D& texture0,
		const TextureView2D& texture1,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
