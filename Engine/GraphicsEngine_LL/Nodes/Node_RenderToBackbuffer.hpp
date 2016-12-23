#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../Camera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../TextureViewPack.hpp"
#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {

// This is a utility node used for testing, makes it easier to render a node's output on the screen.
class RenderToBackbuffer :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<RenderTargetView, RenderTargetPack>,
	virtual public exc::OutputPortConfig<>
{
public:
	RenderToBackbuffer(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto backBuffer = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			auto sourceTexture = this->GetInput<1>().Get();
			this->GetInput<1>().Clear();

			GraphicsCommandList cmdList = context.GetGraphicsCommandList();
			RenderScene(backBuffer, sourceTexture.srv, cmdList);
			result.AddCommandList(std::move(cmdList));

			return result;
		} });
	}

protected:
	VertexBuffer m_fsq;
	IndexBuffer m_fsqIndices;

	GraphicsContext m_graphicsContext;

protected:
	Binder m_binder;
	BindParameter m_texBindParam;
	std::unique_ptr<gxapi::IPipelineState> m_PSO;

private:
	void RenderScene(
		RenderTargetView& rtv,
		Texture2DSRV& texture,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
