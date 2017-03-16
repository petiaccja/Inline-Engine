#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../PerspectiveCamera.hpp"
#include "../ConstBufferHeap.hpp"
#include "../PipelineTypes.hpp"
#include "../GraphicsContext.hpp"

#include "GraphicsApi_LL/IPipelineState.hpp"
#include "GraphicsApi_LL/IGxapiManager.hpp"


namespace inl::gxeng::nodes {


class RenderToBackBuffer :
	virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<>
{
public:
	RenderToBackBuffer(gxapi::IGraphicsApi* graphicsApi);

	void Update() override {}
	void Notify(exc::InputPortBase* sender) override {}
	void InitGraphics(const GraphicsContext& context) override;

	Task GetTask() override {
		return Task({ [this](const ExecutionContext& context) {
			ExecutionResult result;

			auto& swapChainAccessContext = static_cast<const SwapChainAccessContext&>(context);
			auto& backBuffer = *swapChainAccessContext.GetBackBuffer();

			auto sourceTexture = this->GetInput<0>().Get();
			this->GetInput<0>().Clear();

			GraphicsCommandList cmdList = context.GetGraphicsCommandList();
			Render(backBuffer, sourceTexture.QueryRead(), cmdList);
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
	void Render(
		RenderTargetView2D& rtv,
		const TextureView2D& texture,
		GraphicsCommandList& commandList);
};


} // namespace inl::gxeng::nodes
